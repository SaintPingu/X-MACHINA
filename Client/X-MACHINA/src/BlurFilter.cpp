#include "stdafx.h"
#include "BlurFilter.h"
#include "DXGIMgr.h"

#include "Scene.h"
#include "DescriptorHeap.h"
#include "Shader.h"

BlurFilter::BlurFilter(UINT width, UINT height, DXGI_FORMAT format)
	:
	mWidth(width),
	mHeight(height),
	mFormat(format),
	mSigma(2.5f)
{
}

ID3D12Resource* BlurFilter::Resource()
{
	return mBlurMap0.Get();
}

void BlurFilter::Create()
{
	mHorzBlurShader = std::make_unique<HorzBlurShader>();
	mHorzBlurShader->Create();

	mVertBlurShader = std::make_unique<VertBlurShader>();
	mVertBlurShader->Create();

	CreateResources();
	CreateDescriptors();
}

void BlurFilter::OnResize(UINT width, UINT height)
{
	if ((mWidth != width) || (mHeight != height)) {
		mWidth = width;
		mHeight = height;

		CreateResources();
		CreateDescriptors();
	}
}

void BlurFilter::Execute(ID3D12Resource* input, int blurCount)
{
	// 총 가중치 개수는 홀수이어야 중앙을 기준으로 할 수 있다.
	auto weights = CalculateGaussWeights(mSigma);
	int blurRadius = (int)weights.size() / 2;

	// 컴퓨트 루트 시그니처 연결
	cmdList->SetComputeRootSignature(scene->GetComputeRootSignature().Get());

	// 블러 반지름과 가중치 값 연결
	cmdList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	cmdList->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);

	// input(후면 버퍼)을 mBlurMap0에 복사한다. 
	// 초기에 mBlurMap0은 input(후면 버퍼) 텍스처를 가진 SRV(읽기 전용)로 사용된다.
	// 초기에 mBlurMap1은 mBlurMap0의 텍스처를 흐린 값을 저장하기 위한 UAV(쓰기 전용)로 사용된다.
	D3DUtil::ResourceTransition(input, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	D3DUtil::ResourceTransition(mBlurMap0.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->CopyResource(mBlurMap0.Get(), input);
	D3DUtil::ResourceTransition(mBlurMap0.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DUtil::ResourceTransition(mBlurMap1.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 1Pass : read mBlurMap0 and write mBlurMap1
	// 2Pass : read mBlurMap1 and write mBlurMap0 
	for (int i = 0; i < blurCount; ++i) {
#pragma region Horizontal Blur Pass
		mHorzBlurShader->Set();
		cmdList->SetComputeRootDescriptorTable(scene->GetComputeRootParamIndex(RootParam::Read), mBlur0GpuSrv);
		cmdList->SetComputeRootDescriptorTable(scene->GetComputeRootParamIndex(RootParam::Write), mBlur1GpuUav);

		UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
		cmdList->Dispatch(numGroupsX, mHeight, 1);

		D3DUtil::ResourceTransition(mBlurMap0.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		D3DUtil::ResourceTransition(mBlurMap1.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
#pragma endregion

#pragma region Vertical Blur Pass
		mVertBlurShader->Set();
		cmdList->SetComputeRootDescriptorTable(scene->GetComputeRootParamIndex(RootParam::Read), mBlur1GpuUav);
		cmdList->SetComputeRootDescriptorTable(scene->GetComputeRootParamIndex(RootParam::Write), mBlur0GpuSrv);

		UINT numGroupsY = (UINT)ceilf(mHeight / 256.0f);
		cmdList->Dispatch(mWidth, numGroupsY, 1);

		D3DUtil::ResourceTransition(mBlurMap0.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
		D3DUtil::ResourceTransition(mBlurMap1.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
#pragma endregion
	}
}

void BlurFilter::CopyResource(ID3D12Resource* input)
{
	D3DUtil::ResourceTransition(mBlurMap0.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_SOURCE);
	D3DUtil::ResourceTransition(input, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	
	cmdList->CopyResource(input, mBlurMap0.Get());

	D3DUtil::ResourceTransition(mBlurMap0.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
	D3DUtil::ResourceTransition(mBlurMap1.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
}

std::vector<float> BlurFilter::CalculateGaussWeights(float sigma)
{
	float twoSigma2 = 2.f * sigma * sigma;

	int blurRadius = (int)ceil(2.f * sigma);

	assert(blurRadius <= mMaxBlurRadius);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.f;

	for (int i = -blurRadius; i <= blurRadius; ++i) {
		float x = (float)i;

		weights[i + blurRadius] = expf(-x * x / twoSigma2);

		weightSum += weights[i + blurRadius];
	}

	for (int i = 0; i < weights.size(); ++i) {
		weights[i] /= weightSum;
	}

	return weights;
}

void BlurFilter::CreateDescriptors()
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
	
	// 서술자를 생성하고 핸들을 저장한다.
	mBlur0GpuSrv = scene->GetDescHeap()->CreateShaderResourceView(mBlurMap0.Get(), &srvDesc);
	mBlur0GpuUav = scene->GetDescHeap()->CreateUnorderedAccessView(mBlurMap0.Get(), &uavDesc);

	mBlur1GpuSrv = scene->GetDescHeap()->CreateShaderResourceView(mBlurMap1.Get(), &srvDesc);
	mBlur1GpuUav = scene->GetDescHeap()->CreateUnorderedAccessView(mBlurMap1.Get(), &uavDesc);
}

void BlurFilter::CreateResources()
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
		IID_PPV_ARGS(&mBlurMap0)));

	THROW_IF_FAILED(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mBlurMap1)));
}