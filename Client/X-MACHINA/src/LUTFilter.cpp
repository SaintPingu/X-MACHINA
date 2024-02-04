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

ID3D12Resource* LUTFilter::Resource()
{
	return mOutput.Get();
}

void LUTFilter::Create()
{
	mLUTShader = std::make_unique<LUTShader>();
	mLUTShader->Create();

	CreateResources();
	CreateDescriptors();
}

void LUTFilter::Execute(ID3D12Resource* input)
{
	mLUTShader->Set();
	cmdList->SetComputeRootSignature(scene->GetComputeRootSignature().Get());

	mElapsedTime += DeltaTime();
	cmdList->SetComputeRoot32BitConstants(0, 1, &mElapsedTime, 0);
	D3DUtil::ResourceTransition(input, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	D3DUtil::ResourceTransition(mInput.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->CopyResource(mInput.Get(), input);
	D3DUtil::ResourceTransition(mInput.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DUtil::ResourceTransition(mOutput.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	cmdList->SetComputeRootDescriptorTable(scene->GetComputeRootParamIndex(RootParam::LUT0), mLUT0GpuSrv);
	cmdList->SetComputeRootDescriptorTable(scene->GetComputeRootParamIndex(RootParam::LUT1), mLUT1GpuSrv);
	cmdList->SetComputeRootDescriptorTable(scene->GetComputeRootParamIndex(RootParam::Read), mInputGpuSrv);
	cmdList->SetComputeRootDescriptorTable(scene->GetComputeRootParamIndex(RootParam::Write), mOutputGpuUav);

	UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
	cmdList->Dispatch(numGroupsX, mHeight, 1);

	D3DUtil::ResourceTransition(mInput.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON);
	D3DUtil::ResourceTransition(mOutput.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void LUTFilter::CopyResource(ID3D12Resource* input)
{
	D3DUtil::ResourceTransition(mOutput.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE);
	D3DUtil::ResourceTransition(input, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

	cmdList->CopyResource(input, mOutput.Get());

	D3DUtil::ResourceTransition(mOutput.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
}

void LUTFilter::CreateDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = mFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = mFormat;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	mLUT0GpuSrv = scene->GetTexture("LUT_RGB")->GetGpuDescriptorHandle();
	mLUT1GpuSrv = scene->GetTexture("LUT_FoggyNight")->GetGpuDescriptorHandle();
	mInputGpuSrv = scene->GetDescHeap()->CreateShaderResourceView(mInput.Get(), &srvDesc);
	mOutputGpuUav = scene->GetDescHeap()->CreateUnorderedAccessView(mOutput.Get(), &uavDesc);
}

void LUTFilter::CreateResources()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	THROW_IF_FAILED(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mInput)));

	THROW_IF_FAILED(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mOutput)));
}
