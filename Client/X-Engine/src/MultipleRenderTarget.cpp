#include "EnginePch.h"
#include "MultipleRenderTarget.h"
#include "DXGIMgr.h"
#include "FrameResource.h"
#include "ResourceMgr.h"

#include "Texture.h"

void MultipleRenderTarget::Create(GroupType groupType, std::vector<RenderTarget>&& rts, sptr<Texture> dsTexture, Vec4 clearValue)
{
	mGroupType = groupType;
	mRts = std::move(rts);
	mRtCnt = static_cast<UINT>(mRts.size());
	mViewport.resize(mRtCnt);
	mScissorRect.resize(mRtCnt);

	if (dsTexture)
		mTextureDs = dsTexture;

	assert(mMaxRtCnt >= mRtCnt);

	// 렌더 타겟 배열이 없는 경우(그림자) 깊이 텍스처를 사용한다.
	if (mRts.empty()) {
		mViewport.emplace_back(0.f, 0.f, dsTexture->GetWidth(), dsTexture->GetHeight(), 0.f, 1.f);
		mScissorRect.emplace_back(0, 0, static_cast<LONG>(dsTexture->GetWidth()), static_cast<LONG>(dsTexture->GetHeight()));

		mTargetToResource[0] = CD3DX12_RESOURCE_BARRIER::Transition(dsTexture->GetResource().Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);

		mResourceToTarget[0] = CD3DX12_RESOURCE_BARRIER::Transition(dsTexture->GetResource().Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		return;
	}

	for (int i = 0; i < mRts.size(); ++i) {
		mViewport[i] = D3D12_VIEWPORT{ 0.f, 0.f, mRts[i].Target->GetWidth(), mRts[i].Target->GetHeight(), 0.f, 1.f };
		mScissorRect[i] = D3D12_RECT{0, 0, static_cast<LONG>(mRts[i].Target->GetWidth()), static_cast<LONG>(mRts[i].Target->GetHeight())};
	}

	// window resize를 할 때 Create함수가 다시 불리기 때문에 RTV 힙이 이미 존재하는지 검사한다.
	if (!mRtvHeap) {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.NumDescriptors = mRtCnt;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;
		DEVICE->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mRtvHeap));
	}

	// 리소스에 대하여 RTV와 SRV를 생성한다.
	mRtvHeapBegin = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < mRtCnt; ++i) {
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { mRtvHeapBegin.ptr + i * DXGIMgr::I->GetRtvDescriptorIncSize() };
		mRts[i].Target->SetRtvGpuDescriptorHandle(rtvHandle);
		std::array<float, 4> clearColor{ clearValue.x, clearValue.y, clearValue.z, clearValue.w };
		mRts[i].ClearColor = clearColor;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;
		rtvDesc.Format = mRts[i].Target->GetResource()->GetDesc().Format;

		switch (groupType)
		{
		case GroupType::SwapChain:
			break;
		case GroupType::GBuffer:	
		case GroupType::Lighting:
		case GroupType::Ssao:
		case GroupType::Bloom:
		case GroupType::DynamicEnvironment:
			DXGIMgr::I->CreateShaderResourceView(mRts[i].Target.get());
			break;
		case GroupType::OffScreen:
			DXGIMgr::I->CreateShaderResourceView(mRts[i].Target.get());
			DXGIMgr::I->CreateUnorderedAccessView(mRts[i].Target.get());
			break;
		}

		DEVICE->CreateRenderTargetView(mRts[i].Target->GetResource().Get(), &rtvDesc, mRts[i].Target->GetRtvCpuDescriptorHandle());
	}

	// 전이 장벽을 설정한다.
	for (UINT i = 0; i < mRtCnt; ++i) {
		mTargetToResource[i] = CD3DX12_RESOURCE_BARRIER::Transition(mRts[i].Target->GetResource().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		mResourceToTarget[i] = CD3DX12_RESOURCE_BARRIER::Transition(mRts[i].Target->GetResource().Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}

void MultipleRenderTarget::OMSetRenderTargets()
{
	CMD_LIST->RSSetViewports(1, &mViewport[0]);
	CMD_LIST->RSSetScissorRects(1, &mScissorRect[0]);

	CMD_LIST->OMSetRenderTargets(mRtCnt, &mRtvHeapBegin, TRUE, (mTextureDs) ? &mTextureDs->GetDsvCpuDescriptorHandle() : nullptr);
}

void MultipleRenderTarget::OMSetRenderTargets(UINT count, UINT index)
{
	CMD_LIST->RSSetViewports(1, &mViewport[index]);
	CMD_LIST->RSSetScissorRects(1, &mScissorRect[index]);

	CMD_LIST->OMSetRenderTargets(count, (mRtCnt != 0) ? &mRts[index].Target->GetRtvCpuDescriptorHandle() : nullptr, FALSE, 
		(mTextureDs) ? &mTextureDs->GetDsvCpuDescriptorHandle() : nullptr);
}

void MultipleRenderTarget::ClearRenderTargetView(float depthClearValue)
{
	// RTV를 클리어 하기 전 렌더 타겟 텍스처를 렌더 타겟으로 설정한다.
	WaitResourceToTarget();

	// 모든 렌더 타겟 텍스처들을 클리어한다.
	for (UINT i = 0; i < mRtCnt; ++i) {
		CMD_LIST->ClearRenderTargetView(mRts[i].Target->GetRtvCpuDescriptorHandle(), mRts[i].ClearColor.data(), 0, nullptr);
	}

	if (mTextureDs) {
		// 사용할 깊이 버퍼도 클리어한다.
		CMD_LIST->ClearDepthStencilView(mTextureDs->GetDsvCpuDescriptorHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depthClearValue, 0, 0, nullptr);
	}
}

void MultipleRenderTarget::ClearRenderTargetView(UINT index, float depthClearValue)
{
	WaitResourceToTarget(index);

	// index 해당 렌더 타겟만 클리어한다.
	CMD_LIST->ClearRenderTargetView(mRts[index].Target->GetRtvCpuDescriptorHandle(), mRts[index].ClearColor.data(), 0, nullptr);

	if (mTextureDs) {
		CMD_LIST->ClearDepthStencilView(mTextureDs->GetDsvCpuDescriptorHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depthClearValue, 0, 0, nullptr);
	}
}

void MultipleRenderTarget::WaitTargetToResource()
{
	// 모든 렌더 타겟을 렌더 타겟 상태에서 리소스로 변경한다.
	CMD_LIST->ResourceBarrier(max(mRtCnt, 1), mTargetToResource.data());
}

void MultipleRenderTarget::WaitResourceToTarget()
{
	// 모든 렌더 타겟을 리소스 상태에서 렌더 타겟 상태로 변경한다.
	CMD_LIST->ResourceBarrier(max(mRtCnt, 1), mResourceToTarget.data());
}

void MultipleRenderTarget::WaitTargetToResource(UINT index)
{
	// 하나의 렌더 타겟을 렌더 타겟 상태에서 리소스로 변경한다.
	CMD_LIST->ResourceBarrier(1, &mTargetToResource[index]);
}

void MultipleRenderTarget::WaitResourceToTarget(UINT index)
{
	// 하나의 렌더 타겟을 리소스 상태에서 렌더 타겟 상태로 변경한다.
	CMD_LIST->ResourceBarrier(1, &mResourceToTarget[index]);
}

void MultipleRenderTarget::ReleaseRenderTargets()
{
	for (int i = 0; i < mRts.size(); ++i) {
		ResourceMgr::I->Remove<Texture>("SwapChainTarget_" + std::to_string(i));
	}

	mRts.clear();
}
