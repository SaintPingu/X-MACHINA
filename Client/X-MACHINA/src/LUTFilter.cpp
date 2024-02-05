#include "stdafx.h"
#include "LUTFilter.h"
#include "DXGIMgr.h"

#include "Scene.h"
#include "DescriptorHeap.h"
#include "Shader.h"
#include "Texture.h"
#include "Timer.h"

LUTFilter::LUTFilter(UINT width, UINT height, DXGI_FORMAT format)
	:
	mWidth(width),
	mHeight(height),
	mFormat(format)
{
}

void LUTFilter::Create()
{
	mLUTShader = std::make_unique<LUTShader>();
	mLUTShader->Create();

	CreateResources();
	CreateDescriptors();
}

UINT LUTFilter::Execute(rsptr<Texture> input)
{
	mLUTShader->Set();
	cmdList->SetComputeRootSignature(scene->GetComputeRootSignature().Get());

	mElapsedTime += DeltaTime();
	cmdList->SetComputeRoot32BitConstants(0, 1, &mElapsedTime, 0);
	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	cmdList->SetComputeRootDescriptorTable(scene->GetRootParamIndex(RootParam::LUT0), scene->GetTexture("LUT_RGB")->GetGpuDescriptorHandle());
	cmdList->SetComputeRootDescriptorTable(scene->GetRootParamIndex(RootParam::LUT1), scene->GetTexture("LUT_RGB")->GetGpuDescriptorHandle());
	cmdList->SetComputeRootDescriptorTable(scene->GetRootParamIndex(RootParam::Read), input->GetGpuDescriptorHandle());
	cmdList->SetComputeRootDescriptorTable(scene->GetRootParamIndex(RootParam::Write), mOutput->GetUavGpuDescriptorHandle());

	UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
	cmdList->Dispatch(numGroupsX, mHeight, 1);

	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

	return mOutput->GetGpuDescriptorHandleIndex();
}

void LUTFilter::CreateDescriptors()
{
	scene->CreateShaderResourceView(mOutput.get());
	scene->CreateUnorderedAccessView(mOutput.get());
}

void LUTFilter::CreateResources()
{
	mOutput = std::make_shared<Texture>(D3DResource::Texture2D);
	mOutput->CreateTexture(
		mWidth,
		mHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON);
}
