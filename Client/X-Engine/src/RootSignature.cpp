#include "EnginePch.h"
#include "RootSignature.h"
#include "DXGIMgr.h"

#pragma region RootSignature
RootSignature::RootSignature()
{
	constexpr int kMaxRangeSize = 32;
	mRanges.resize(kMaxRangeSize);
}

void RootSignature::Push(RootParam param, D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility, UINT num32BitValues)
{
	assert(paramType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);

	D3D12_ROOT_PARAMETER rootParam{};

	rootParam.ParameterType            = paramType;
	rootParam.ShaderVisibility         = visibility;

	if (num32BitValues > 0) {
		rootParam.Constants.Num32BitValues = num32BitValues;
		rootParam.Constants.ShaderRegister = shaderRegister;
		rootParam.Constants.RegisterSpace = registerSpace;
	}
	else {
		rootParam.Descriptor.ShaderRegister = shaderRegister;
		rootParam.Descriptor.RegisterSpace = registerSpace;
	}

	mParams.push_back(rootParam);

	ParamMapping(param);
}

void RootSignature::PushTable(RootParam param, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT registerSpace, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility)
{
	D3D12_DESCRIPTOR_RANGE range{};

	range.RangeType                         = rangeType;
	range.NumDescriptors                    = numDescriptors;
	range.BaseShaderRegister                = shaderRegister;
	range.RegisterSpace						= registerSpace;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	mRanges.push_back(range);


	D3D12_ROOT_PARAMETER rootParam{};

	rootParam.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.DescriptorTable.NumDescriptorRanges = 1;
	rootParam.DescriptorTable.pDescriptorRanges   = &mRanges.back();

	switch (mType)
	{
	case RootSignatureType::Graphics:
		rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		break;
	case RootSignatureType::Compute:
		rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		break;
	default:
		assert(1);
		break;
	}

	mParams.push_back(rootParam);

	ParamMapping(param);
}

void RootSignature::ParamMapping(RootParam param)
{
	mParamMap[param] = (UINT)mParams.size() - 1;
}
#pragma endregion




#pragma region GraphicsRootSignature
GraphicsRootSignature::GraphicsRootSignature()
{
	mType = RootSignatureType::Graphics;
}

void GraphicsRootSignature::Create()
{
	auto staticSamplers = GetStaticSamplers();

	// flags
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

	// description
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters     = (UINT)mParams.size();
	rootSignatureDesc.pParameters       = mParams.data();
	rootSignatureDesc.NumStaticSamplers = (UINT)staticSamplers.size();
	rootSignatureDesc.pStaticSamplers	= staticSamplers.data();
	rootSignatureDesc.Flags             = rootSignatureFlags;

	// serialize
	ComPtr<ID3DBlob> signatureBlob{};
	ComPtr<ID3DBlob> errBlob{};
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errBlob);
	PrintErrorBlob(errBlob);

	// create
	device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
}

void GraphicsRootSignature::CreateDefaultGraphicsRootSignature()
{
	// 자주 사용되는 것을 앞에 배치할 것. (빠른 메모리 접근)
	Push(RootParam::Object, D3D12_ROOT_PARAMETER_TYPE_CBV, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
	Push(RootParam::Pass, D3D12_ROOT_PARAMETER_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
	Push(RootParam::PostPass, D3D12_ROOT_PARAMETER_TYPE_CBV, 2, 0, D3D12_SHADER_VISIBILITY_ALL);
	Push(RootParam::Collider, D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 3, 0, D3D12_SHADER_VISIBILITY_ALL, 16);
	Push(RootParam::SkinMesh, D3D12_ROOT_PARAMETER_TYPE_CBV, 4, 0, D3D12_SHADER_VISIBILITY_ALL);
	Push(RootParam::Ssao, D3D12_ROOT_PARAMETER_TYPE_CBV, 5, 0, D3D12_SHADER_VISIBILITY_ALL);
	Push(RootParam::SsaoBlur, D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 6, 0, D3D12_SHADER_VISIBILITY_ALL, 2);

	// 머티리얼은 space1을 사용하여 t0을 TextureCube와 같이 사용하여도 겹치지 않음
	Push(RootParam::Instancing, D3D12_ROOT_PARAMETER_TYPE_SRV, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	Push(RootParam::Material, D3D12_ROOT_PARAMETER_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	Push(RootParam::Particle, D3D12_ROOT_PARAMETER_TYPE_SRV, 0, 2, D3D12_SHADER_VISIBILITY_ALL);

	// TextureCube 형식을 제외한 모든 텍스처들은 Texture2D 배열에 저장된다.
	PushTable(RootParam::SkyBox, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, gkDescHeapSkyBoxCount, D3D12_SHADER_VISIBILITY_PIXEL);
	PushTable(RootParam::Texture, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, gkDescHeapSrvCount, D3D12_SHADER_VISIBILITY_PIXEL);

	Create();
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 8> GraphicsRootSignature::GetStaticSamplers()
{
	// 자주 사용되는 샘플러들

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	const CD3DX12_STATIC_SAMPLER_DESC depthMapSam(
		7, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,
		0,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp, shadow, depthMapSam
	};
}
#pragma endregion




#pragma region ComputeRootSignature
ComputeRootSignature::ComputeRootSignature()
{
	mType = RootSignatureType::Compute;
}

void ComputeRootSignature::CreateDefaultComputeRootSignature()
{
	// 가중치 루트 상수 (b0)
	Push(RootParam::Weight, D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 0, 0, D3D12_SHADER_VISIBILITY_ALL, 12);

	// 읽기 전용 SRV 서술자 테이블 (t0)
	PushTable(RootParam::Read, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 0, 1, D3D12_SHADER_VISIBILITY_ALL);

	// 읽기 전용 SRV 서술자 테이블 (t1, t2)
	PushTable(RootParam::LUT0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
	PushTable(RootParam::LUT1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 1, D3D12_SHADER_VISIBILITY_ALL);

	// 쓰기 전용 UAV 서술자 테이블 (u0)
	PushTable(RootParam::Write, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 0, 1, D3D12_SHADER_VISIBILITY_ALL);

	Create();
}

void ComputeRootSignature::CreateParticleComputeRootSignature()
{
	// 파티클 시스템 데이터 (t0, space0)
	Push(RootParam::ParticleSystem, D3D12_ROOT_PARAMETER_TYPE_SRV, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
	// 파티클 시스템 데이터 (u0, space1)
	Push(RootParam::ParticleShared, D3D12_ROOT_PARAMETER_TYPE_UAV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
	// 인풋 파티클 데이터 (u0, space2)
	Push(RootParam::ComputeParticle, D3D12_ROOT_PARAMETER_TYPE_UAV, 0, 2, D3D12_SHADER_VISIBILITY_ALL);
	// 파티클 시스템 인덱스 (b0)
	Push(RootParam::ParticleIndex, D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 0, 0, D3D12_SHADER_VISIBILITY_ALL, 1);

	Create();
}

void ComputeRootSignature::Create()
{
	// flags
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// description
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters     = (UINT)mParams.size();
	rootSignatureDesc.pParameters       = mParams.data();
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers   = nullptr;
	rootSignatureDesc.Flags             = rootSignatureFlags;

	// serialize
	ComPtr<ID3DBlob> signatureBlob{};
	ComPtr<ID3DBlob> errBlob{};
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errBlob);
	PrintErrorBlob(errBlob);

	// create
	device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
}
#pragma endregion
