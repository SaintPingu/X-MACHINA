#pragma once

#pragma region Include
#include "Resources.h"
#pragma endregion

#pragma region EnumClass
enum class ShaderType : UINT8 {
	LDR = 0,
	HDR,
	Deferred,
	Lighting,
	Compute,
};

enum class RasterizerType : UINT8 {
	Cull_None,
	Cull_Front,
	Cull_Back,
	WireFrame,
};

enum class DepthStencilType : UINT8 {
	Less,
	Less_Equal,
	Greater,
	Greater_Equal,
	No_DepthTest,
	No_DepthTest_No_Write,
	Less_No_Write,
};

enum class BlendType : UINT8 {
	Default,
	Alpha_Blend,
	One_To_One_Blend,
};

enum class InputLayoutType : UINT8 {
	None,
	Default,
	ColorInst,
	Wire,
};
#pragma endregion

#pragma region Structure
struct ShaderInfo {
	ShaderType				ShaderType = ShaderType::LDR;
	RasterizerType			RasterizerType = RasterizerType::Cull_Back;
	DepthStencilType		DepthStencilType = DepthStencilType::Less;
	BlendType				BlendType = BlendType::Default;
	InputLayoutType			InputLayoutType = InputLayoutType::Default;
	D3D_PRIMITIVE_TOPOLOGY	TopologyType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

struct ShaderPath
{
	const std::wstring	VS = L"";
	const std::wstring	PS = L"";
	const std::wstring	CS = L"";
};
#pragma endregion

#pragma region Class
class Shader : public Resource {
private:
	bool mIsClosed{ false };	// PipelineState생성을 종료하고 불필요 메모리를 해제했는가?

	ShaderInfo mInfo;

	std::vector<ComPtr<ID3D12PipelineState>>	mPipelineStates{};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC			mGraphicsPipelineStateDesc{};
	D3D12_COMPUTE_PIPELINE_STATE_DESC			mComputePipelineStateDesc{};

	ComPtr<ID3DBlob> mVSBlob{};
	ComPtr<ID3DBlob> mPSBlob{};
	ComPtr<ID3DBlob> mCSBlob{};

public:
	Shader() : Resource(ResourceType::Shader) {};
	virtual ~Shader();

public:
	static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(D3D_PRIMITIVE_TOPOLOGY topology);

public:
	void Load(ShaderInfo info = ShaderInfo{}, ShaderPath path = ShaderPath{}, bool isClose = true);

	void CreateGraphicsShader(bool isClose = true);
	void CreateComputeShader(bool isClose = true);
	void Set(int pipelineStateIndex = 0);

private:
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	D3D12_RASTERIZER_DESC CreateRasterizerState();
	D3D12_BLEND_DESC CreateBlendState();
	D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	// PipelineState 생성을 중단하고, 생성 과정에 발생한 불필요한 메모리를 해제한다.
	void Close();
};
#pragma endregion
