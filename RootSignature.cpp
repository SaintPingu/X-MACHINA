#include "stdafx.h"
#include "RootSignature.h"
#include "DXGIMgr.h"

CGraphicsRootSignature::CGraphicsRootSignature()
{
	mRanges.resize(32);
}

void CGraphicsRootSignature::ParamMapping(RootParam param)
{
	mParamMap[param] = mParams.size() - 1;
}

void CGraphicsRootSignature::Push(RootParam param, D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility, UINT num32BitValues, UINT registerSpace)
{
	assert(paramType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);

	D3D12_ROOT_PARAMETER rootParam{};

	rootParam.ParameterType = paramType;
	rootParam.Constants.Num32BitValues = num32BitValues;
	rootParam.Constants.ShaderRegister = shaderRegister;
	rootParam.Constants.RegisterSpace = registerSpace;
	rootParam.ShaderVisibility = visibility;

	mParams.emplace_back(rootParam);

	ParamMapping(param);
}

void CGraphicsRootSignature::PushTable(RootParam param, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility)
{
	D3D12_DESCRIPTOR_RANGE range{};

	range.RangeType = rangeType;
	range.NumDescriptors = numDescriptors;
	range.BaseShaderRegister = shaderRegister;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	mRanges.emplace_back(range);


	D3D12_ROOT_PARAMETER rootParam{};

	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.DescriptorTable.NumDescriptorRanges = 1;
	rootParam.DescriptorTable.pDescriptorRanges = &mRanges.back();
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	mParams.emplace_back(rootParam);

	ParamMapping(param);
}

RComPtr<ID3D12RootSignature> CGraphicsRootSignature::Create()
{
	// sampler
	D3D12_STATIC_SAMPLER_DESC samplerDescs[2]{};
	samplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDescs[0].MipLODBias = 0;
	samplerDescs[0].MaxAnisotropy = 1;
	samplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[0].MinLOD = 0;
	samplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDescs[0].ShaderRegister = 0;
	samplerDescs[0].RegisterSpace = 0;
	samplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	samplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDescs[1].MipLODBias = 0;
	samplerDescs[1].MaxAnisotropy = 1;
	samplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDescs[1].MinLOD = 0;
	samplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	samplerDescs[1].ShaderRegister = 1;
	samplerDescs[1].RegisterSpace = 0;
	samplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// flags
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// description
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = mParams.size();
	rootSignatureDesc.pParameters = mParams.data();
	rootSignatureDesc.NumStaticSamplers = _countof(samplerDescs);
	rootSignatureDesc.pStaticSamplers = samplerDescs;
	rootSignatureDesc.Flags = rootSignatureFlags;

	// serialize
	ComPtr<ID3DBlob> signatureBlob{};
	ComPtr<ID3DBlob> errBlob{};
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errBlob);
	PrintErrorBlob(errBlob);

	// create
	device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));

	return mRootSignature;
}
