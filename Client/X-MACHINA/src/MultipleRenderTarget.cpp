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

	// ���� Ÿ�� �迭�� ���� ���(�׸���) ���� �ؽ�ó�� ����Ѵ�.
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

	// window resize�� �� �� Create�Լ��� �ٽ� �Ҹ��� ������ RTV ���� �̹� �����ϴ��� �˻��Ѵ�.
	if (!mRtvHeap) {
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.NumDescriptors = mRtCnt;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;
		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mRtvHeap));
	}

	// ���ҽ��� ���Ͽ� RTV�� SRV�� �����Ѵ�.
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
	// RTV�� Ŭ���� �ϱ� �� ���� Ÿ�� �ؽ�ó�� ���� Ÿ������ �����Ѵ�.
	WaitResourceToTarget();

	// ��� ���� Ÿ�� �ؽ�ó���� Ŭ�����Ѵ�.
	for (UINT i = 0; i < mRtCnt; ++i) {
		cmdList->ClearRenderTargetView(mRts[i].RtvHandle, mRts[i].ClearColor.data(), 0, nullptr);
	}

	// ����� ���� ���۵� Ŭ�����Ѵ�.
	cmdList->ClearDepthStencilView(mDsvHeapBegin, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
}

void MultipleRenderTarget::ClearRenderTargetView(UINT index)
{
	WaitResourceToTarget(index);

	// index �ش� ���� Ÿ�ٸ� Ŭ�����Ѵ�.
	cmdList->ClearRenderTargetView(mRts[index].RtvHandle, mRts[index].ClearColor.data(), 0, nullptr);

	cmdList->ClearDepthStencilView(mDsvHeapBegin, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
}

void MultipleRenderTarget::WaitTargetToResource()
{
	// ��� ���� Ÿ���� ���� Ÿ�� ���¿��� ���ҽ��� �����Ѵ�.
	cmdList->ResourceBarrier(max(mRtCnt, 1), mTargetToResource.data());
}

void MultipleRenderTarget::WaitResourceToTarget()
{
	// ��� ���� Ÿ���� ���ҽ� ���¿��� ���� Ÿ�� ���·� �����Ѵ�.
	cmdList->ResourceBarrier(max(mRtCnt, 1), mResourceToTarget.data());
}

void MultipleRenderTarget::WaitTargetToResource(UINT index)
{
	// �ϳ��� ���� Ÿ���� ���� Ÿ�� ���¿��� ���ҽ��� �����Ѵ�.
	cmdList->ResourceBarrier(1, &mTargetToResource[index]);
}

void MultipleRenderTarget::WaitResourceToTarget(UINT index)
{
	// �ϳ��� ���� Ÿ���� ���ҽ� ���¿��� ���� Ÿ�� ���·� �����Ѵ�.
	cmdList->ResourceBarrier(1, &mResourceToTarget[index]);
}

void MultipleRenderTarget::ReleaseRenderTargets()
{
	if (!mRts.empty()) {
		mRts.clear();
	}
}
