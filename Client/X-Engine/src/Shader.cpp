#include "EnginePch.h"
#include "Shader.h"
#include "DXGIMgr.h"

#define READ_COMPILED_SHADER


#pragma region Shader
Shader::~Shader()
{
	assert(mIsClosed);
}

void Shader::LoadPSCD(ShaderInfo info, ShaderPath path, bool isClose)
{
	mInfo = info;

	if (!path.CS.empty()) {
		mCSBlob = D3DUtil::ReadCompiledShaderFile(path.CS);

		CreateComputeShader(isClose);
	}
	else {
		if (!path.VS.empty())
			mVSBlob = D3DUtil::ReadCompiledShaderFile(path.VS);
		if (!path.PS.empty())
			mPSBlob = D3DUtil::ReadCompiledShaderFile(path.PS);
		if (!path.GS.empty())
			mGSBlob = D3DUtil::ReadCompiledShaderFile(path.GS);
		CreateGraphicsShader(isClose);
	}
}

void Shader::CreateGraphicsShader(bool isClose)
{
	assert(!mIsClosed);

	mGraphicsPipelineStateDesc.VS = mVSBlob ? D3D12_SHADER_BYTECODE{ (BYTE*)(mVSBlob->GetBufferPointer()), mVSBlob->GetBufferSize() } : D3D12_SHADER_BYTECODE{};
	mGraphicsPipelineStateDesc.PS = mPSBlob ? D3D12_SHADER_BYTECODE{ (BYTE*)(mPSBlob->GetBufferPointer()), mPSBlob->GetBufferSize() } : D3D12_SHADER_BYTECODE{};
	mGraphicsPipelineStateDesc.GS = mGSBlob ? D3D12_SHADER_BYTECODE{ (BYTE*)(mGSBlob->GetBufferPointer()), mGSBlob->GetBufferSize() } : D3D12_SHADER_BYTECODE{};
	mGraphicsPipelineStateDesc.pRootSignature			= DXGIMgr::I->GetGraphicsRootSignature().Get();
	mGraphicsPipelineStateDesc.RasterizerState			= CreateRasterizerState();
	mGraphicsPipelineStateDesc.BlendState				= CreateBlendState();
	mGraphicsPipelineStateDesc.DepthStencilState		= CreateDepthStencilState();
	mGraphicsPipelineStateDesc.InputLayout				= CreateInputLayout();
	mGraphicsPipelineStateDesc.PrimitiveTopologyType	= GetTopologyType(mInfo.TopologyType);
	mGraphicsPipelineStateDesc.SampleMask				= UINT_MAX;
	mGraphicsPipelineStateDesc.DSVFormat				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	mGraphicsPipelineStateDesc.SampleDesc.Count			= 1;
	mGraphicsPipelineStateDesc.Flags					= D3D12_PIPELINE_STATE_FLAG_NONE;

	// 쉐이더 타입에 따라서 RTV 포맷을 다르게 한다. 기본은 Forward이다.
	switch (mInfo.ShaderType) {
	case ShaderType::LDR:
		mGraphicsPipelineStateDesc.NumRenderTargets = 1;
		mGraphicsPipelineStateDesc.RTVFormats[0]	= DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case ShaderType::HDR:
		mGraphicsPipelineStateDesc.NumRenderTargets = 1;
		mGraphicsPipelineStateDesc.RTVFormats[0]	= DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case ShaderType::Shadow:
		mGraphicsPipelineStateDesc.NumRenderTargets = 0;
		mGraphicsPipelineStateDesc.RTVFormats[0]	= DXGI_FORMAT_UNKNOWN;
		break;
	case ShaderType::Deferred:
		mGraphicsPipelineStateDesc.NumRenderTargets = GBufferCount;
		mGraphicsPipelineStateDesc.RTVFormats[0]	= DXGI_FORMAT_R32G32B32A32_FLOAT;
		mGraphicsPipelineStateDesc.RTVFormats[1]	= DXGI_FORMAT_R32G32B32A32_FLOAT;
		mGraphicsPipelineStateDesc.RTVFormats[2]	= DXGI_FORMAT_R8G8B8A8_UNORM;
		mGraphicsPipelineStateDesc.RTVFormats[3]	= DXGI_FORMAT_R8G8B8A8_UNORM;
		mGraphicsPipelineStateDesc.RTVFormats[4]	= DXGI_FORMAT_R8G8_UNORM;
		mGraphicsPipelineStateDesc.RTVFormats[5]	= DXGI_FORMAT_R16_UNORM;
		break;
	case ShaderType::Lighting:
		mGraphicsPipelineStateDesc.NumRenderTargets = LightingCount;
		mGraphicsPipelineStateDesc.RTVFormats[0]	= DXGI_FORMAT_R16G16B16A16_FLOAT;
		mGraphicsPipelineStateDesc.RTVFormats[1]	= DXGI_FORMAT_R16G16B16A16_FLOAT;
		mGraphicsPipelineStateDesc.RTVFormats[2]	= DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case ShaderType::Ssao:
		mGraphicsPipelineStateDesc.NumRenderTargets = SsaoCount;
		mGraphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R16_UNORM;
		mGraphicsPipelineStateDesc.RTVFormats[1] = DXGI_FORMAT_R16_UNORM;
		break;
	default:
		break;
	}

	if (mPipelineStates.empty()) {
		mPipelineStates.resize(1);
	}

	HRESULT hResult = DEVICE->CreateGraphicsPipelineState(&mGraphicsPipelineStateDesc, IID_PPV_ARGS(&mPipelineStates[0]));
	AssertHResult(hResult);

	mIsClosed = false;
	if (isClose) {
		Close();
	}
}

void Shader::CreateComputeShader(bool isClose)
{
	assert(!mIsClosed);

	switch (mInfo.ShaderType) {
	case ShaderType::Compute:
		mComputePipelineStateDesc.pRootSignature = DXGIMgr::I->GetComputeRootSignature().Get();
		break;
	case ShaderType::Particle:
		mComputePipelineStateDesc.pRootSignature = DXGIMgr::I->GetParticleComputeRootSignature().Get();
		break;
	default:
		break;
	}

	mComputePipelineStateDesc.CS = { (BYTE*)(mCSBlob->GetBufferPointer()), mCSBlob->GetBufferSize() };
	mComputePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	if (mPipelineStates.empty()) {
		mPipelineStates.resize(1);
	}

	HRESULT hResult = DEVICE->CreateComputePipelineState(&mComputePipelineStateDesc, IID_PPV_ARGS(&mPipelineStates[0]));
	AssertHResult(hResult);

	mIsClosed = false;
	if (isClose) {
		Close();
	}
}

void Shader::Set(int pipelineStateIndex)
{
	assert(mPipelineStates.size() >= pipelineStateIndex + 1);

	CMD_LIST->SetPipelineState(mPipelineStates[pipelineStateIndex].Get());
}

D3D12_INPUT_LAYOUT_DESC Shader::CreateInputLayout()
{
	UINT nInputElementDescs{};
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs{};

	switch (mInfo.InputLayoutType) {
	case InputLayoutType::None:
		nInputElementDescs = 0;
		inputElementDescs = nullptr;
		break;
	case InputLayoutType::Default:
		nInputElementDescs = 7;
		inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
		inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputElementDescs[1] = { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		break;
	case InputLayoutType::ColorInst:
		nInputElementDescs = 2;
		inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
		inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		inputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		break;
	case InputLayoutType::Wire:
		nInputElementDescs = 1;
		inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
		inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		break;
	}

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;
	return inputLayoutDesc;
}

D3D12_RASTERIZER_DESC Shader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	switch (mInfo.RasterizerType) {
	case RasterizerType::Cull_None:
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		break;
	case RasterizerType::Cull_Front:
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
		break;
	case RasterizerType::Cull_Back:
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		break;
	case RasterizerType::WireFrame:
		rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		break;
	case RasterizerType::DepthBias:
		rasterizerDesc.DepthBias = 10000;
		rasterizerDesc.DepthBiasClamp = 0.f;
		rasterizerDesc.SlopeScaledDepthBias = 1.f;
		break;
	default:
		break;
	}

	return rasterizerDesc;
}

D3D12_DEPTH_STENCIL_DESC Shader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	
	switch (mInfo.DepthStencilType) {
	case DepthStencilType::Less:
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		break;
	case DepthStencilType::Less_Equal:
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		break;
	case DepthStencilType::Greater:
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
		break;
	case DepthStencilType::Greater_Equal:
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		break;
	case DepthStencilType::No_DepthTest:
		depthStencilDesc.DepthEnable = FALSE;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		break;
	case DepthStencilType::No_DepthTest_No_Write:
		depthStencilDesc.DepthEnable = FALSE;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		break;
	case DepthStencilType::Less_No_Write:
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		break;
	default:
		break;
	}

	return depthStencilDesc;
}

D3D12_BLEND_DESC Shader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	switch (mInfo.BlendType)
	{
	case BlendType::Default:
		blendDesc.RenderTarget[0].BlendEnable = FALSE;
		blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		break;
	case BlendType::Alpha_Blend:
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		break;
	case BlendType::One_To_One_Blend:
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	}

	return blendDesc;
}

void Shader::Close()
{
	mIsClosed	= true;
	mVSBlob		= nullptr;
	mPSBlob		= nullptr;
	mCSBlob		= nullptr;

	if (mGraphicsPipelineStateDesc.InputLayout.pInputElementDescs) {
		delete[] mGraphicsPipelineStateDesc.InputLayout.pInputElementDescs;
	}
}
#pragma endregion


D3D12_PRIMITIVE_TOPOLOGY_TYPE Shader::GetTopologyType(D3D_PRIMITIVE_TOPOLOGY topology)
{
	switch (topology)
	{
	case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
	case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
	case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
	case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST:
	case D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	default:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}
}