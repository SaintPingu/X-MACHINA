#include "stdafx.h"
#include "MultipleRenderTarget.h"
#include "DXGIMgr.h"
#include "FrameResource.h"

#include "Scene.h"
#include "Texture.h"

rsptr<Texture> MultipleRenderTarget::GetTexture(UINT index) const
{
	return mRts[index].Target;
}

rsptr<Texture> MultipleRenderTarget::GetTexture(GBuffer index) const
{
	return mRts[static_cast<UINT8>(index)].Target;
}

void MultipleRenderTarget::Create(GroupType groupType, std::vector<RenderTarget>&& rts, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
{
	mGroupType = groupType;
	mRts = std::move(rts);
	mRtCnt = static_cast<UINT>(mRts.size());
	mDsvHandle = dsvHandle;

	assert(mMaxRtCnt >= mRtCnt);

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

		scene->CreateShaderResourceView(mRts[i].Target.get());
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
	// set viewports
	// set scissorRects
	cmdList->OMSetRenderTargets(mRtCnt, &mRtvHeapBegin, TRUE, &mDsvHandle);
}

void MultipleRenderTarget::OMSetRenderTargets(UINT count, UINT index)
{
	// set viewports
	// set scissorRects
	cmdList->OMSetRenderTargets(count, &mRts[index].RtvHandle, FALSE, &mDsvHandle);
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
	cmdList->ClearDepthStencilView(mDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
}

void MultipleRenderTarget::ClearRenderTargetView(UINT index)
{
	WaitResourceToTarget(index);

	// index �ش� ���� Ÿ�ٸ� Ŭ�����Ѵ�.
	cmdList->ClearRenderTargetView(mRts[index].RtvHandle, mRts[index].ClearColor.data(), 0, nullptr);

	cmdList->ClearDepthStencilView(mDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
}

void MultipleRenderTarget::WaitTargetToResource()
{
	// ��� ���� Ÿ���� ���� Ÿ�� ���¿��� ���ҽ��� �����Ѵ�.
	cmdList->ResourceBarrier(mRtCnt, mTargetToResource.data());
}

void MultipleRenderTarget::WaitResourceToTarget()
{
	// ��� ���� Ÿ���� ���ҽ� ���¿��� ���� Ÿ�� ���·� �����Ѵ�.
	cmdList->ResourceBarrier(mRtCnt, mResourceToTarget.data());
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
