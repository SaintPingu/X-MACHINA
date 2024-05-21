#include "EnginePch.h"
#include "LUTFilter.h"
#include "DXGIMgr.h"

#include "ResourceMgr.h"
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
	CreateResources();
	CreateDescriptors();
}

UINT LUTFilter::Execute(rsptr<Texture> input)
{
	RESOURCE<Shader>("LUT")->Set();

	mElapsedTime += DeltaTime();
	DWORD filterOption = DXGIMgr::I->GetFilterOption();
	CMD_LIST->SetComputeRoot32BitConstants(0, 1, &mElapsedTime, 0);
	CMD_LIST->SetComputeRoot32BitConstants(0, 1, &filterOption, 1);
	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// LUT 텍스처는 BC7 형식으로 압축해야 포토샵 LUT와 최대한 똑같은 색상을 추출할 수 있다.
	CMD_LIST->SetComputeRootDescriptorTable(DXGIMgr::I->GetComputeRootParamIndex(RootParam::LUT0), RESOURCE<Texture>("LUT_RGB")->GetGpuDescriptorHandle());
	CMD_LIST->SetComputeRootDescriptorTable(DXGIMgr::I->GetComputeRootParamIndex(RootParam::LUT1), RESOURCE<Texture>("LUT_RGB")->GetGpuDescriptorHandle());
	CMD_LIST->SetComputeRootDescriptorTable(DXGIMgr::I->GetComputeRootParamIndex(RootParam::Read), input->GetGpuDescriptorHandle());
	CMD_LIST->SetComputeRootDescriptorTable(DXGIMgr::I->GetComputeRootParamIndex(RootParam::Write), mOutput->GetUavGpuDescriptorHandle());

	UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
	CMD_LIST->Dispatch(numGroupsX, mHeight, 1);

	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

	return mOutput->GetSrvIdx();
}

void LUTFilter::CreateDescriptors()
{
	DXGIMgr::I->CreateShaderResourceView(mOutput.get());
	DXGIMgr::I->CreateUnorderedAccessView(mOutput.get());
}

void LUTFilter::CreateResources()
{
	mOutput = std::make_shared<Texture>();
	mOutput->Create(mWidth, mHeight, 
		DXGI_FORMAT_R8G8B8A8_UNORM, 
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, 
		D3D12_RESOURCE_STATE_COMMON);
}
