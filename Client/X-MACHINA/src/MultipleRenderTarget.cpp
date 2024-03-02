#include "stdafx.h"
#include "MultipleRenderTarget.h"
#include "DXGIMgr.h"
#include "FrameResource.h"

#include "Texture.h"

void MultipleRenderTarget::Create(GroupType groupType, std::vector<RenderTarget>&& rts, sptr<Texture> dsTexture)
{
	mGroupType = groupType;
	mRts = std::move(rts);
	mRtCnt = static_cast<UINT>(mRts.size());
	mDsvHeapBegin = dsTexture->GetDsvCpuDescriptorHandle();

	assert(mMaxRtCnt >= mRtCnt);

	// 렌더 타겟 배열이 없는 경우(그림자) 깊이 텍스처를 사용한다.
	if (mRts.empty()) {
		mViewport = D3D12_VIEWPORT{ 0.f, 0.f, dsTexture->GetWidth(), dsTexture->GetHeight(), 0.f, 1.f };
		mScissorRect = D3D12_RECT{ 0, 0, static_cast<LONG>(dsTexture->GetWidth()), static_cast<LONG>(dsTexture->GetHeight()) };

		mTargetToResource[0] = CD3DX12_RESOURCE_BARRIER::Transition(dsTexture->GetResource().Get(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);

		mResourceToTarget[0] = CD3DX12_RESOURCE_BARRIER::Transition(dsTexture->GetResource().Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		return;
	}

	mViewport = D3D12_VIEWPORT{ 0.f, 0.f, mRts[0].Target->GetWidth(), mRts[0].Target->GetHeight(), 0.f, 1.f };
	mScissorRect = D3D12_RECT{ 0, 0, static_cast<LONG>(mRts[0].Target->GetWidth()), static_cast<LONG>(mRts[0].Target->GetHeight()) };

	// window resize를 할 때 Create함수가 다시 불리기 때문에 RTV 힙이 이미 존재하는지 검사한다.
	if (!mRtvHeap) {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.NumDescriptors = mRtCnt;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;
		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mRtvHeap));
	}

	// 리소스에 대하여 RTV와 SRV를 생성한다.
	mRtvHeapBegin = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < mRtCnt; ++i) {
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { mRtvHeapBegin.ptr + i * dxgi->GetRtvDescriptorIncSize() };
		mRts[i].RtvHandle = rtvHandle;

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
			dxgi->CreateShaderResourceView(mRts[i].Target.get());
			break;
		case GroupType::OffScreen:
			dxgi->CreateShaderResourceView(mRts[i].Target.get());
			dxgi->CreateUnorderedAccessView(mRts[i].Target.get());
			break;
		}

		device->CreateRenderTargetView(mRts[i].Target->GetResource().Get(), &rtvDesc, mRts[i].RtvHandle);
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
	cmdList->RSSetViewports(1, &mViewport);
	cmdList->RSSetScissorRects(1, &mScissorRect);

	cmdList->OMSetRenderTargets(mRtCnt, &mRtvHeapBegin, TRUE, &mDsvHeapBegin);
}

void MultipleRenderTarget::OMSetRenderTargets(UINT count, UINT index)
{
	cmdList->RSSetViewports(1, &mViewport);
	cmdList->RSSetScissorRects(1, &mScissorRect);

	cmdList->OMSetRenderTargets(count, (mRtCnt != 0) ? &mRts[index].RtvHandle : nullptr, FALSE, &mDsvHeapBegin);
}

void MultipleRenderTarget::ClearRenderTargetView()
{
	// RTV를 클리어 하기 전 렌더 타겟 텍스처를 렌더 타겟으로 설정한다.
	WaitResourceToTarget();

	// 모든 렌더 타겟 텍스처들을 클리어한다.
	for (UINT i = 0; i < mRtCnt; ++i) {
		cmdList->ClearRenderTargetView(mRts[i].RtvHandle, mRts[i].ClearColor.data(), 0, nullptr);
	}

	// 사용할 깊이 버퍼도 클리어한다.
	cmdList->ClearDepthStencilView(mDsvHeapBegin, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
}

void MultipleRenderTarget::ClearRenderTargetView(UINT index)
{
	WaitResourceToTarget(index);

	// index 해당 렌더 타겟만 클리어한다.
	cmdList->ClearRenderTargetView(mRts[index].RtvHandle, mRts[index].ClearColor.data(), 0, nullptr);

	cmdList->ClearDepthStencilView(mDsvHeapBegin, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
}

void MultipleRenderTarget::WaitTargetToResource()
{
	// 모든 렌더 타겟을 렌더 타겟 상태에서 리소스로 변경한다.
	cmdList->ResourceBarrier(max(mRtCnt, 1), mTargetToResource.data());
}

void MultipleRenderTarget::WaitResourceToTarget()
{
	// 모든 렌더 타겟을 리소스 상태에서 렌더 타겟 상태로 변경한다.
	cmdList->ResourceBarrier(max(mRtCnt, 1), mResourceToTarget.data());
}

void MultipleRenderTarget::WaitTargetToResource(UINT index)
{
	// 하나의 렌더 타겟을 렌더 타겟 상태에서 리소스로 변경한다.
	cmdList->ResourceBarrier(1, &mTargetToResource[index]);
}

void MultipleRenderTarget::WaitResourceToTarget(UINT index)
{
	// 하나의 렌더 타겟을 리소스 상태에서 렌더 타겟 상태로 변경한다.
	cmdList->ResourceBarrier(1, &mResourceToTarget[index]);
}

void MultipleRenderTarget::ReleaseRenderTargets()
{
	if (!mRts.empty()) {
		mRts.clear();
	}
}
