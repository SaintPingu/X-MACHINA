#include "stdafx.h"
#include "MultipleRenderTarget.h"
#include "DXGIMgr.h"
#include "FrameResource.h"

#include "Scene.h"
#include "Texture.h"

MultipleRenderTarget::MultipleRenderTarget()
{
	mRtvCnt = (UINT)dxgi->GetRtvFormats().size() - 1;
	mRtvFormats = dxgi->GetRtvFormats().data() + 1;
}

void MultipleRenderTarget::SetMRTTsPassConstants(PassConstants& passConstants)
{
	for (UINT i = 0; i < mRtvCnt; ++i) {
		passConstants.MRTTsIndices[i] = mTextures[i]->GetGpuDescriptorHandleIndex();
	}
}

void MultipleRenderTarget::CreateResourcesAndRtvsSrvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
	CreateTextureResources();
	CreateSrvs();
	CreateRtvs(rtvHandle);
}

void MultipleRenderTarget::OnPrepareRenderTargets(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle)
{
	constexpr size_t kRenderTargetCnt = 1;

	size_t resourceCnt = mTextures.size();
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> allRtvHandle(kRenderTargetCnt + resourceCnt);

	// 후면 버퍼 (SV_TARGET[0])
	allRtvHandle.front() = rtvHandles[0];
	cmdList->ClearRenderTargetView(rtvHandles[0], Colors::White, 0, NULL);

	// (SV_TARGET[1] ~ SV_TARGET[n])
	for (size_t i = 0; i < resourceCnt; ++i) {
		D3DUtil::ResourceTransition(mTextures[i]->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHandles[i];
		cmdList->ClearRenderTargetView(rtvHandle, Colors::White, 0, NULL);
		allRtvHandle[kRenderTargetCnt + i] = rtvHandle;
	}

	cmdList->OMSetRenderTargets((UINT)(kRenderTargetCnt + resourceCnt), allRtvHandle.data(), FALSE, dsvHandle);
}

void MultipleRenderTarget::OnPostRenderTarget()
{
	for (const auto& texture : mTextures) {
		D3DUtil::ResourceTransition(texture->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
	}
}

void MultipleRenderTarget::CreateTextureResources()
{
	mTextures.resize(mRtvCnt);

	D3D12_CLEAR_VALUE clearValue = { DXGI_FORMAT_R8G8B8A8_UNORM, { 1.f, 1.f, 1.f, 1.f } };
	for (UINT i = 0; i < mRtvCnt; ++i)
	{
		clearValue.Format = mRtvFormats[i];
		mTextures[i] = std::make_shared<Texture>(D3DResource::Texture2D);
		mTextures[i]->CreateTextureResource(gkFrameBufferWidth, gkFrameBufferHeight, mRtvFormats[i], D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &clearValue);
	}
}

void MultipleRenderTarget::CreateSrvs()
{
	for (UINT i = 0; i < mRtvCnt; ++i) {
		scene->CreateShaderResourceView(mTextures[i].get(), 0);
	}
}

void MultipleRenderTarget::CreateRtvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	mRtvHandles.resize(mRtvCnt);

	for (UINT i = 0; i < mRtvCnt; ++i) {
		rtvDesc.Format = mRtvFormats[i];
		ComPtr<ID3D12Resource> textureResource = mTextures[i]->GetResource();
		device->CreateRenderTargetView(textureResource.Get(), &rtvDesc, rtvHandle);
		mRtvHandles[i] = rtvHandle;
		rtvHandle.ptr += dxgi->GetRtvDescriptorIncSize();
	}
}

