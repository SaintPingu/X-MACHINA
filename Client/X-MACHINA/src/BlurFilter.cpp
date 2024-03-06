#include "stdafx.h"
#include "BlurFilter.h"
#include "DXGIMgr.h"

#include "ResourceMgr.h"
#include "Scene.h"
#include "DescriptorHeap.h"
#include "Shader.h"
#include "Texture.h"

BlurFilter::BlurFilter(UINT width, UINT height, DXGI_FORMAT format)
	:
	mWidth(width),
	mHeight(height),
	mFormat(format),
	mSigma(2.5f)
{
}

void BlurFilter::Create()
{
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

UINT BlurFilter::Execute(rsptr<Texture> input, int blurCount)
{
	// 총 가중치 개수는 홀수이어야 중앙을 기준으로 할 수 있다.
	auto weights = CalculateGaussWeights(mSigma);
	int blurRadius = (int)weights.size() / 2;

	// 컴퓨트 루트 시그니처 연결
	cmdList->SetComputeRootSignature(dxgi->GetComputeRootSignature().Get());

	// 블러 반지름과 가중치 값 연결
	cmdList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	cmdList->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);

	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	
	// 1Pass : read mBlurMap0 and write mBlurMap1
	// 2Pass : read mBlurMap1 and write mBlurMap0 
	for (int i = 0; i < blurCount; ++i) {
#pragma region Horizontal Blur Pass
		res->Get<Shader>("HorzBlur")->Set();
		cmdList->SetComputeRootDescriptorTable(dxgi->GetComputeRootParamIndex(RootParam::Read), input->GetGpuDescriptorHandle());
		cmdList->SetComputeRootDescriptorTable(dxgi->GetComputeRootParamIndex(RootParam::Write), mOutput->GetUavGpuDescriptorHandle());

		UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
		cmdList->Dispatch(numGroupsX, mHeight, 1);

		D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
#pragma endregion

#pragma region Vertical Blur Pass
		res->Get<Shader>("VertBlur")->Set();
		cmdList->SetComputeRootDescriptorTable(dxgi->GetComputeRootParamIndex(RootParam::Read), mOutput->GetGpuDescriptorHandle());
		cmdList->SetComputeRootDescriptorTable(dxgi->GetComputeRootParamIndex(RootParam::Write), input->GetUavGpuDescriptorHandle());

		UINT numGroupsY = (UINT)ceilf(mHeight / 256.0f);
		cmdList->Dispatch(mWidth, numGroupsY, 1);

		D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
		D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
#pragma endregion
	}
	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

	return input->GetGpuDescriptorHandleIndex();
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
	dxgi->CreateShaderResourceView(mOutput.get());
	dxgi->CreateUnorderedAccessView(mOutput.get());
}

void BlurFilter::CreateResources()
{
	mOutput = std::make_shared<Texture>();
	mOutput->Create(mWidth, mHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON);
}