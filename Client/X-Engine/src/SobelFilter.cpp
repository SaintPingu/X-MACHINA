#include "EnginePch.h"
#include "SobelFilter.h"
#include "DXGIMgr.h"

#include "Texture.h"
#include "Shader.h"
#include "ResourceMgr.h"

SobelFilter::SobelFilter(UINT width, UINT height)
	:
	mWidth(width),
	mHeight(height)
{
}

void SobelFilter::Create()
{
	CreateResource();
}

void SobelFilter::OnResize(UINT width, UINT height)
{
	if ((mWidth != width) || (mHeight != height)) {
		mWidth = width;
		mHeight = height;

		CreateResource();
	}
}

UINT SobelFilter::Execute(rsptr<Texture> input)
{
	RESOURCE<Shader>("Sobel")->Set();

	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	CMD_LIST->SetComputeRootDescriptorTable(DXGIMgr::I->GetComputeRootParamIndex(RootParam::Read), input->GetGpuDescriptorHandle());
	CMD_LIST->SetComputeRootDescriptorTable(DXGIMgr::I->GetComputeRootParamIndex(RootParam::Write), mOutput->GetUavGpuDescriptorHandle());

	UINT numGroupsX = (UINT)ceilf(mWidth / 16.0f);
	UINT numGroupsY = (UINT)ceilf(mHeight / 16.0f);
	CMD_LIST->Dispatch(numGroupsX, numGroupsY, 1);

	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

	return mOutput->GetSrvIdx();
}

void SobelFilter::CreateResource()
{
	mOutput = std::make_shared<Texture>();
	mOutput->Create(mWidth, mHeight,
		DXGI_FORMAT_R8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON);

	DXGIMgr::I->CreateShaderResourceView(mOutput.get());
	DXGIMgr::I->CreateUnorderedAccessView(mOutput.get());
}
