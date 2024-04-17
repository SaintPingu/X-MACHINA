#include "EnginePch.h"
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
	auto weights = Filter::CalcGaussWeights(mSigma);
	int blurRadius = (int)weights.size() / 2;

	// 블러 반지름과 가중치 값 연결
	CMD_LIST->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	CMD_LIST->SetComputeRoot32BitConstants(0, (UINT)weights.size(), weights.data(), 1);

	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	
	// 1Pass : read mBlurMap0 and write mBlurMap1
	// 2Pass : read mBlurMap1 and write mBlurMap0 
	for (int i = 0; i < blurCount; ++i) {
#pragma region Horizontal Blur Pass
		RESOURCE<Shader>("HorzBlur")->Set();
		CMD_LIST->SetComputeRootDescriptorTable(DXGIMgr::I->GetComputeRootParamIndex(RootParam::Read), input->GetGpuDescriptorHandle());
		CMD_LIST->SetComputeRootDescriptorTable(DXGIMgr::I->GetComputeRootParamIndex(RootParam::Write), mOutput->GetUavGpuDescriptorHandle());

		UINT numGroupsX = (UINT)ceilf(mWidth / 256.0f);
		CMD_LIST->Dispatch(numGroupsX, mHeight, 1);

		D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
#pragma endregion

#pragma region Vertical Blur Pass
		RESOURCE<Shader>("VertBlur")->Set();
		CMD_LIST->SetComputeRootDescriptorTable(DXGIMgr::I->GetComputeRootParamIndex(RootParam::Read), mOutput->GetGpuDescriptorHandle());
		CMD_LIST->SetComputeRootDescriptorTable(DXGIMgr::I->GetComputeRootParamIndex(RootParam::Write), input->GetUavGpuDescriptorHandle());

		UINT numGroupsY = (UINT)ceilf(mHeight / 256.0f);
		CMD_LIST->Dispatch(mWidth, numGroupsY, 1);

		D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);
		D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
#pragma endregion
	}
	D3DUtil::ResourceTransition(input->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON);
	D3DUtil::ResourceTransition(mOutput->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);

	return input->GetSrvIdx();
}

void BlurFilter::CreateDescriptors()
{
	DXGIMgr::I->CreateShaderResourceView(mOutput.get());
	DXGIMgr::I->CreateUnorderedAccessView(mOutput.get());
}

void BlurFilter::CreateResources()
{
	mOutput = std::make_shared<Texture>();
	mOutput->Create(mWidth, mHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON);
}