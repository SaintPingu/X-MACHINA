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

	// ���� Ÿ�� �迭�� ���� ���(�׸���) ���� �ؽ�ó�� ����Ѵ�.
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

	// window resize�� �� �� Create�Լ��� �ٽ� �Ҹ��� ������ RTV ���� �̹� �����ϴ��� �˻��Ѵ�.
	if (!mRtvHeap) {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.NumDescriptors = mRtCnt;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;
		DEVICE->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mRtvHeap));
	}

	// ���ҽ��� ���Ͽ� RTV�� SRV�� �����Ѵ�.
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

	// ���� �庮�� �����Ѵ�.
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
	// RTV�� Ŭ���� �ϱ� �� ���� Ÿ�� �ؽ�ó�� ���� Ÿ������ �����Ѵ�.
	WaitResourceToTarget();

	// ��� ���� Ÿ�� �ؽ�ó���� Ŭ�����Ѵ�.
	for (UINT i = 0; i < mRtCnt; ++i) {
		CMD_LIST->ClearRenderTargetView(mRts[i].Target->GetRtvCpuDescriptorHandle(), mRts[i].ClearColor.data(), 0, nullptr);
	}

	if (mTextureDs) {
		// ����� ���� ���۵� Ŭ�����Ѵ�.
		CMD_LIST->ClearDepthStencilView(mTextureDs->GetDsvCpuDescriptorHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depthClearValue, 0, 0, nullptr);
	}
}

void MultipleRenderTarget::ClearRenderTargetView(UINT index, float depthClearValue)
{
	WaitResourceToTarget(index);

	// index �ش� ���� Ÿ�ٸ� Ŭ�����Ѵ�.
	CMD_LIST->ClearRenderTargetView(mRts[index].Target->GetRtvCpuDescriptorHandle(), mRts[index].ClearColor.data(), 0, nullptr);

	if (mTextureDs) {
		CMD_LIST->ClearDepthStencilView(mTextureDs->GetDsvCpuDescriptorHandle(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depthClearValue, 0, 0, nullptr);
	}
}

void MultipleRenderTarget::WaitTargetToResource()
{
	// ��� ���� Ÿ���� ���� Ÿ�� ���¿��� ���ҽ��� �����Ѵ�.
	CMD_LIST->ResourceBarrier(max(mRtCnt, 1), mTargetToResource.data());
}

void MultipleRenderTarget::WaitResourceToTarget()
{
	// ��� ���� Ÿ���� ���ҽ� ���¿��� ���� Ÿ�� ���·� �����Ѵ�.
	CMD_LIST->ResourceBarrier(max(mRtCnt, 1), mResourceToTarget.data());
}

void MultipleRenderTarget::WaitTargetToResource(UINT index)
{
	// �ϳ��� ���� Ÿ���� ���� Ÿ�� ���¿��� ���ҽ��� �����Ѵ�.
	CMD_LIST->ResourceBarrier(1, &mTargetToResource[index]);
}

void MultipleRenderTarget::WaitResourceToTarget(UINT index)
{
	// �ϳ��� ���� Ÿ���� ���ҽ� ���¿��� ���� Ÿ�� ���·� �����Ѵ�.
	CMD_LIST->ResourceBarrier(1, &mResourceToTarget[index]);
}

void MultipleRenderTarget::ReleaseRenderTargets()
{
	for (int i = 0; i < mRts.size(); ++i) {
		ResourceMgr::I->Remove<Texture>("SwapChainTarget_" + std::to_string(i));
	}

	mRts.clear();
}
