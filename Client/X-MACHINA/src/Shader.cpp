#include "stdafx.h"
#include "Shader.h"
#include "DXGIMgr.h"
#include "FrameResource.h"

#include "Scene.h"
#include "Texture.h"


//#define READ_COMPILED_SHADER





#pragma region Shader
Shader::~Shader()
{
	assert(mIsClosed);
}

void Shader::Create(DXGI_FORMAT dsvFormat, bool isClose)
{
	assert(!mIsClosed);

	mPipelineStateDesc.pRootSignature        = scene->GetGraphicsRootSignature().Get();
	mPipelineStateDesc.VS                    = CreateVertexShader();
	mPipelineStateDesc.PS                    = CreatePixelShader();
	mPipelineStateDesc.RasterizerState       = CreateRasterizerState();
	mPipelineStateDesc.BlendState            = CreateBlendState();
	mPipelineStateDesc.DepthStencilState     = CreateDepthStencilState();
	mPipelineStateDesc.InputLayout           = CreateInputLayout();
	mPipelineStateDesc.SampleMask            = UINT_MAX;
	mPipelineStateDesc.PrimitiveTopologyType = GetPrimitiveType();
	mPipelineStateDesc.DSVFormat             = dsvFormat;
	mPipelineStateDesc.SampleDesc.Count      = 1;
	mPipelineStateDesc.Flags                 = D3D12_PIPELINE_STATE_FLAG_NONE;

	const auto& rtvFormats = dxgi->GetRtvFormats();
	mPipelineStateDesc.NumRenderTargets = (UINT)rtvFormats.size();
	for (UINT i = 0; i < mPipelineStateDesc.NumRenderTargets; ++i) {
		mPipelineStateDesc.RTVFormats[i] = rtvFormats[i];
	}

	if (mPipelineStates.empty()) {
		mPipelineStates.resize(1);
	}

	HRESULT hResult = device->CreateGraphicsPipelineState(&mPipelineStateDesc, IID_PPV_ARGS(&mPipelineStates[0]));
	AssertHResult(hResult);

	mIsClosed = false;
	if (isClose) {
		Close();
	}
}

void Shader::Set(int pipelineStateIndex)
{
	SetPipelineState(pipelineStateIndex);
}



void Shader::SetPipelineState(int pipelineStateIndex)
{
	assert(mPipelineStates.size() >= pipelineStateIndex + 1);

	cmdList->SetPipelineState(mPipelineStates[pipelineStateIndex].Get());
}

D3D12_INPUT_LAYOUT_DESC Shader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = nullptr;
	inputLayoutDesc.NumElements = 0;

	return inputLayoutDesc;
}

D3D12_RASTERIZER_DESC Shader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode              = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode              = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias             = 0;
	rasterizerDesc.DepthBiasClamp        = 0.f;
	rasterizerDesc.SlopeScaledDepthBias  = 0.f;
	rasterizerDesc.DepthClipEnable       = TRUE;
	rasterizerDesc.MultisampleEnable     = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount     = 0;
	rasterizerDesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_DEPTH_STENCIL_DESC Shader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable                  = TRUE;
	depthStencilDesc.DepthWriteMask               = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc                    = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable                = FALSE;
	depthStencilDesc.StencilReadMask              = 0x00;
	depthStencilDesc.StencilWriteMask             = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc        = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp       = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp  = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp       = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc         = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_BLEND_DESC Shader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable                 = FALSE;
	blendDesc.IndependentBlendEnable                = FALSE;
	blendDesc.RenderTarget[0].BlendEnable           = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable         = FALSE;
	blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_SHADER_BYTECODE Shader::CreateVertexShader()
{
	throw std::runtime_error("not assigned vertex shader!");
}

D3D12_SHADER_BYTECODE Shader::CreatePixelShader()
{
	throw std::runtime_error("not assigned pixel shader!");
}

void Shader::Close()
{
	mIsClosed = true;
	mVSBlob = nullptr;
	mPSBlob = nullptr;
	if (mPipelineStateDesc.InputLayout.pInputElementDescs) {
		delete[] mPipelineStateDesc.InputLayout.pInputElementDescs;
	}
}
#pragma endregion





#pragma region WireShader
D3D12_INPUT_LAYOUT_DESC WireShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}

D3D12_RASTERIZER_DESC WireShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode              = D3D12_FILL_MODE_WIREFRAME;
	rasterizerDesc.CullMode              = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias             = 0;
	rasterizerDesc.DepthBiasClamp        = 0.f;
	rasterizerDesc.SlopeScaledDepthBias  = 0.f;
	rasterizerDesc.DepthClipEnable       = TRUE;
	rasterizerDesc.MultisampleEnable     = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount     = 0;
	rasterizerDesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_SHADER_BYTECODE WireShader::CreateVertexShader()
{
#ifdef READ_COMPILED_SHADER
	//return Shader::ReadCompiledShaderFile(L"VShader_Wired.cso", mVSBlob);
	return D3DUtil::CompileShaderFile(L"VShader_Wired.hlsl", "VSWired", "vs_5_1", mVSBlob);
#else
	return D3DUtil::CompileShaderFile(L"VShader_Wired.hlsl", "VSWired", "vs_5_1", mVSBlob);
#endif
}

D3D12_SHADER_BYTECODE WireShader::CreatePixelShader()
{
#ifdef READ_COMPILED_SHADER
	//return Shader::ReadCompiledShaderFile(L"PShader_Wired.cso", mPSBlob);
	return D3DUtil::CompileShaderFile(L"PShader_Wired.hlsl", "PSWired", "ps_5_1", mPSBlob);
#else
	return D3DUtil::CompileShaderFile(L"PShader_Wired.hlsl", "PSWired", "ps_5_1", mPSBlob);
#endif  
#pragma endregion

}




#pragma region ColorInstShader
D3D12_INPUT_LAYOUT_DESC ColorInstShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements        = nInputElementDescs;
	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE ColorInstShader::CreateVertexShader()
{
#ifdef READ_COMPILED_SHADER
	return D3DUtil::ReadCompiledShaderFile(L"VShader_Instance.cso", mVSBlob);
#else
	return D3DUtil::CompileShaderFile(L"VShader_Instance.hlsl", "VSInstancing", "vs_5_1", mVSBlob);
#endif
}

D3D12_SHADER_BYTECODE ColorInstShader::CreatePixelShader()
{
#ifdef READ_COMPILED_SHADER
	return D3DUtil::ReadCompiledShaderFile(L"PShader_Instance.cso", mPSBlob);
#else
	return D3DUtil::CompileShaderFile(L"PShader_Instance.hlsl", "PSInstancing", "ps_5_1", mPSBlob);
#endif
}
#pragma endregion






#pragma region TexturedEffectShader
D3D12_INPUT_LAYOUT_DESC TexturedEffectShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };


	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements        = nInputElementDescs;
	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE TexturedEffectShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_TextureInstance.hlsl", "VSTextureInstancing", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE TexturedEffectShader::CreatePixelShader()
{
	return D3DUtil::CompileShaderFile(L"PShader_TextureInstance.hlsl", "PSTextureInstancing", "ps_5_1", mPSBlob);
}
#pragma endregion




#pragma region TexturedShader
D3D12_INPUT_LAYOUT_DESC TexturedShader::CreateInputLayout()
{
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements        = nInputElementDescs;

	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE TexturedShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_Standard.hlsl", "VS_Standard", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE TexturedShader::CreatePixelShader()
{
	//return Shader::CompileShaderFile(L"PShader_Standard.hlsl", "PS_Standard", "ps_5_1", mPSBlob);
	return D3DUtil::CompileShaderFile(L"PShader_MRT.hlsl", "PSTexturedLightingToMultipleRTs", "ps_5_1", mPSBlob);
}
#pragma endregion





#pragma region ObjectInstShader
D3D12_SHADER_BYTECODE ObjectInstShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_StandardInstance.hlsl", "VS_StandardInstance", "vs_5_1", mVSBlob);
}
#pragma endregion






#pragma region TransparentShader
D3D12_DEPTH_STENCIL_DESC TransparentShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable                  = TRUE;
	depthStencilDesc.DepthWriteMask               = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc                    = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable                = FALSE;
	depthStencilDesc.StencilReadMask              = 0xff;
	depthStencilDesc.StencilWriteMask             = 0xff;
	depthStencilDesc.FrontFace.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp       = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp  = D3D12_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp       = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc         = D3D12_COMPARISON_FUNC_ALWAYS;

	return depthStencilDesc;
}
D3D12_BLEND_DESC TransparentShader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable                 = TRUE;
	blendDesc.IndependentBlendEnable                = FALSE;
	blendDesc.RenderTarget[0].BlendEnable           = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable         = FALSE;
	blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}
#pragma endregion





#pragma region TerrainShader
D3D12_INPUT_LAYOUT_DESC TerrainShader::CreateInputLayout()
{
	UINT nInputElementDescs = 4;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[2] = { "UVA", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[3] = { "UVB", 0, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}

D3D12_RASTERIZER_DESC TerrainShader::CreateRasterizerState()
{
	return Shader::CreateRasterizerState();

	// WIREFRAME //
	//D3D12_RASTERIZER_DESC rasterizerDesc{};
	//rasterizerDesc.FillMode              = D3D12_FILL_MODE_WIREFRAME;
	//rasterizerDesc.CullMode              = D3D12_CULL_MODE_NONE;
	//rasterizerDesc.FrontCounterClockwise = FALSE;
	//rasterizerDesc.DepthBias             = 0;
	//rasterizerDesc.DepthBiasClamp        = 0.f;
	//rasterizerDesc.SlopeScaledDepthBias  = 0.f;
	//rasterizerDesc.DepthClipEnable       = TRUE;
	//rasterizerDesc.MultisampleEnable     = FALSE;
	//rasterizerDesc.AntialiasedLineEnable = FALSE;
	//rasterizerDesc.ForcedSampleCount     = 0;
	//rasterizerDesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//return rasterizerDesc;
}

D3D12_SHADER_BYTECODE TerrainShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_Terrain.hlsl", "VSTerrain", "vs_5_1", mVSBlob);
}


D3D12_SHADER_BYTECODE TerrainShader::CreatePixelShader()
{
	return D3DUtil::CompileShaderFile(L"PShader_Terrain.hlsl", "PSTerrain", "ps_5_1", mPSBlob);
}
#pragma endregion





#pragma region SkyBoxShader
D3D12_INPUT_LAYOUT_DESC SkyBoxShader::CreateInputLayout()
{
	UINT nInputElementDescs = 1;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;

	return inputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC SkyBoxShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable                  = TRUE;
	depthStencilDesc.DepthWriteMask               = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc                    = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable                = FALSE;
	depthStencilDesc.StencilReadMask              = 0xff;
	depthStencilDesc.StencilWriteMask             = 0xff;
	depthStencilDesc.FrontFace.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp       = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp  = D3D12_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp       = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc         = D3D12_COMPARISON_FUNC_ALWAYS;

	return depthStencilDesc;
}

D3D12_SHADER_BYTECODE SkyBoxShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_Skybox.hlsl", "VSSkyBox", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE SkyBoxShader::CreatePixelShader()
{
	return D3DUtil::CompileShaderFile(L"PShader_Skybox.hlsl", "PSSkyBox", "ps_5_1", mPSBlob);
}
#pragma endregion






#pragma region WaterShader
D3D12_BLEND_DESC WaterShader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable                 = FALSE;
	blendDesc.IndependentBlendEnable                = FALSE;
	blendDesc.RenderTarget[0].BlendEnable           = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable         = FALSE;
	blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_DEPTH_STENCIL_DESC WaterShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable                  = TRUE;
	depthStencilDesc.DepthWriteMask               = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc                    = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.StencilEnable                = FALSE;
	depthStencilDesc.StencilReadMask              = 0xff;
	depthStencilDesc.StencilWriteMask             = 0xff;
	depthStencilDesc.FrontFace.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp       = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp  = D3D12_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp       = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc         = D3D12_COMPARISON_FUNC_ALWAYS;

	return depthStencilDesc;
}


D3D12_SHADER_BYTECODE WaterShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_Water.hlsl", "VSWater", "vs_5_1", mVSBlob);
}


D3D12_SHADER_BYTECODE WaterShader::CreatePixelShader()
{
	return D3DUtil::CompileShaderFile(L"PShader_Water.hlsl", "PSWater", "ps_5_1", mPSBlob);
}
#pragma endregion






#pragma region PostProcessingShader
void PostProcessingShader::Set(int pipelineStateIndex)
{
	Shader::Set(pipelineStateIndex);
}


D3D12_DEPTH_STENCIL_DESC PostProcessingShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable                  = FALSE;
	depthStencilDesc.DepthWriteMask               = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc                    = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilDesc.StencilEnable                = FALSE;
	depthStencilDesc.StencilReadMask              = 0x00;
	depthStencilDesc.StencilWriteMask             = 0x00;
	depthStencilDesc.FrontFace.StencilFailOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc        = D3D12_COMPARISON_FUNC_NEVER;
	depthStencilDesc.BackFace.StencilFailOp       = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp  = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp       = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc         = D3D12_COMPARISON_FUNC_NEVER;

	return depthStencilDesc;
}

D3D12_SHADER_BYTECODE PostProcessingShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_Post.hlsl", "VSPostProcessing", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE PostProcessingShader::CreatePixelShader()
{
	return D3DUtil::CompileShaderFile(L"PShader_Post.hlsl", "PSPostProcessing", "ps_5_1", mPSBlob);
}
#pragma endregion





#pragma region TextureToScreenShader
D3D12_SHADER_BYTECODE TextureToScreenShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_Rect.hlsl", "VSScreen", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE TextureToScreenShader::CreatePixelShader()
{
	return D3DUtil::CompileShaderFile(L"PShader_Rect.hlsl", "PSScreenRectSamplingTextured", "ps_5_1", mPSBlob);
}
#pragma endregion






#pragma region BillboardShader
D3D12_RASTERIZER_DESC BillboardShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode              = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode              = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias             = -8000;
	rasterizerDesc.DepthBiasClamp        = 0.f;
	rasterizerDesc.SlopeScaledDepthBias  = 0.f;
	rasterizerDesc.DepthClipEnable       = TRUE;
	rasterizerDesc.MultisampleEnable     = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount     = 0;
	rasterizerDesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_BLEND_DESC BillboardShader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable                 = FALSE;
	blendDesc.IndependentBlendEnable                = FALSE;
	blendDesc.RenderTarget[0].BlendEnable           = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable         = FALSE;
	blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_SHADER_BYTECODE BillboardShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_Billboard.hlsl", "VSBillboard", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE BillboardShader::CreatePixelShader()
{
	return D3DUtil::CompileShaderFile(L"PShader_Billboard.hlsl", "PSBillboard", "ps_5_1", mPSBlob);
}




D3D12_SHADER_BYTECODE SpriteShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_Billboard.hlsl", "VSSprite", "vs_5_1", mVSBlob);
}
#pragma endregion






#pragma region CanvasShader
D3D12_INPUT_LAYOUT_DESC CanvasShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
	inputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputElementDescs[1] = { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = nInputElementDescs;
	return inputLayoutDesc;
}

D3D12_RASTERIZER_DESC CanvasShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode              = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode              = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias             = 0;
	rasterizerDesc.DepthBiasClamp        = 0.f;
	rasterizerDesc.SlopeScaledDepthBias  = 0.f;
	rasterizerDesc.DepthClipEnable       = TRUE;
	rasterizerDesc.MultisampleEnable     = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount     = 0;
	rasterizerDesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return rasterizerDesc;
}

D3D12_BLEND_DESC CanvasShader::CreateBlendState()
{
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable                 = TRUE;
	blendDesc.IndependentBlendEnable                = FALSE;
	blendDesc.RenderTarget[0].BlendEnable           = TRUE;
	blendDesc.RenderTarget[0].LogicOpEnable         = FALSE;
	blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return blendDesc;
}

D3D12_SHADER_BYTECODE CanvasShader::CreateVertexShader()
{
	return D3DUtil::CompileShaderFile(L"VShader_Canvas.hlsl", "VSCanvas", "vs_5_1", mVSBlob);
}

D3D12_SHADER_BYTECODE CanvasShader::CreatePixelShader()
{
	return D3DUtil::CompileShaderFile(L"PShader_Canvas.hlsl", "PSCanvas", "ps_5_1", mPSBlob);
}
#pragma endregion




// [ ComputeShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma region ComputeShader
ComputeShader::~ComputeShader()
{
	assert(mIsClosed);
}

void ComputeShader::Create(bool isClose)
{
	assert(!mIsClosed);
	
	mPipelineStateDesc.pRootSignature = scene->GetComputeRootSignature().Get();
	mPipelineStateDesc.CS = CreateComputeShader();
	mPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = device->CreateComputePipelineState(&mPipelineStateDesc, IID_PPV_ARGS(&mPipelineState));
	AssertHResult(hResult);

	mIsClosed = false;
	if (isClose) {
		Close();
	}
}

void ComputeShader::Set()
{
	cmdList->SetPipelineState(mPipelineState.Get());
}

D3D12_SHADER_BYTECODE ComputeShader::CreateComputeShader()
{
	throw std::runtime_error("not assigned compute shader!");
}

void ComputeShader::Close()
{
	mIsClosed = true;
	mCSBlob = nullptr;
}
#pragma endregion




#pragma region BlurShader
D3D12_SHADER_BYTECODE HorzBlurShader::CreateComputeShader()
{
	return D3DUtil::CompileShaderFile(L"CShader_Blur.hlsl", "HorzBlurCS", "cs_5_1", mCSBlob);
}

D3D12_SHADER_BYTECODE VertBlurShader::CreateComputeShader()
{
	return D3DUtil::CompileShaderFile(L"CShader_Blur.hlsl", "VertBlurCS", "cs_5_1", mCSBlob);
}
#pragma endregion


