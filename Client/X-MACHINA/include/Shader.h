#pragma once

#pragma region ClassForwardDecl
class ModelObjectMesh;
class Mesh;
class GameObject;
class GridObject;
class MasterModel;
class Object;
class Material;
class Texture;
class Script_Fragment;
class Script_Bullet;
struct PassConstants;
#pragma endregion

#pragma region EnumClass
enum class ShaderType : UINT8 {
	Forward = 0,
	Deferred,
	Lighting,
	OffScreen,
	Final,
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
	Default,
	ColorInst,
	Wire,
};

struct ShaderInfo {
	ShaderType			ShaderType = ShaderType::Forward;
	RasterizerType		RasterizerType = RasterizerType::Cull_Back;
	DepthStencilType	DepthStencilType = DepthStencilType::Less;
	BlendType			BlendType = BlendType::Default;
	InputLayoutType		InputLayoutType = InputLayoutType::Default;
};
#pragma endregion

#pragma region Class
#pragma region Shader
class Shader {
private:
	bool mIsClosed{ false };	// PipelineState생성을 종료하고 불필요 메모리를 해제했는가?

protected:
	ShaderInfo mInfo;

	std::vector<ComPtr<ID3D12PipelineState>>	mPipelineStates{};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC			mPipelineStateDesc{};	// 마지막으로 설정된 PipelineStateDesc

	ComPtr<ID3DBlob> mVSBlob{};
	ComPtr<ID3DBlob> mPSBlob{};

public:
	Shader(ShaderInfo info = ShaderInfo{}) : mInfo(info) {};
	virtual ~Shader();

public:
	virtual void Create(ShaderType shaderType = ShaderType::Forward, DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT, bool isClose = true);
	virtual void Set(int pipelineStateIndex = 0);

protected:
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType() const { return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; };

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual ComPtr<ID3DBlob> CreateVertexShader();
	virtual ComPtr<ID3DBlob> CreatePixelShader();

	// PipelineState 생성을 중단하고, 생성 과정에 발생한 불필요한 메모리를 해제한다.
	void Close();
};

// [ DeferredShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DeferredShader : public Shader {
public:
	DeferredShader() = default;
	virtual ~DeferredShader() = default;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;

};

// for rendering instancing GameObjects
class ColorInstShader : public DeferredShader {
public:
	ColorInstShader() = default;
	virtual ~ColorInstShader() = default;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;
};

// for rendering instancing GameObjects that has texture
class ObjectInstShader : public DeferredShader {
public:
	ObjectInstShader() = default;
	virtual ~ObjectInstShader() = default;

protected:
	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
};

// for rendering Terrain
class TerrainShader : public DeferredShader {
public:
	TerrainShader() = default;
	virtual ~TerrainShader() = default;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;

	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;

};

// for rendering instancing GameObjects
class WaterShader : public DeferredShader {
public:
	WaterShader() = default;
	virtual ~WaterShader() = default;

protected:
	virtual D3D12_BLEND_DESC CreateBlendState() override;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;
};

// for rendering instancing GameObjects
class LightingShader : public Shader {
public:
	LightingShader() = default;
	virtual ~LightingShader() = default;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;
};

// [ ForwardShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ForwardShader : public Shader {
public:
	ForwardShader() = default;
	virtual ~ForwardShader() = default;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;

};

// for rendering wireframe
class WireShader : public ForwardShader {
public:
	WireShader() = default;
	virtual ~WireShader() = default;

protected:
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType() const override { return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;

	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;
};

// for rendering transparent GameObjects
class TransparentShader : public ForwardShader {
public:
	TransparentShader() = default;
	virtual ~TransparentShader() = default;

protected:
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;
	virtual D3D12_BLEND_DESC CreateBlendState() override;
};

// for rendering SkyBox
class SkyBoxShader : public Shader {
public:
	SkyBoxShader() = default;
	virtual ~SkyBoxShader() = default;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;
};

#pragma region BillboardShader
// for rendering billboard GameObjects
class BillboardShader : public ForwardShader {
public:
	BillboardShader() = default;
	virtual ~BillboardShader() = default;

protected:
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;
	virtual D3D12_BLEND_DESC CreateBlendState() override;

	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;
};

// for rendering sprite GameObjects
class SpriteShader : public BillboardShader {
public:
	SpriteShader() = default;
	virtual ~SpriteShader() = default;

protected:
	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
};
#pragma endregion

// for rendering UI (2D plane)
class CanvasShader : public ForwardShader {
public:
	CanvasShader() = default;
	virtual ~CanvasShader() = default;

public:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;
	virtual D3D12_BLEND_DESC CreateBlendState() override;

	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;
};

// for post processing
class FinalShader : public ForwardShader {
public:
	FinalShader() = default;
	virtual ~FinalShader() = default;

public:
	virtual void Set(int pipelineStateIndex = 0) override;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;
};
#pragma endregion

// 후면 버퍼 대신 그려줄 쉐이더
class OffScreenShader : public FinalShader {
public:
	OffScreenShader() = default;
	virtual ~OffScreenShader() = default;

protected:
	virtual ComPtr<ID3DBlob> CreatePixelShader() override;
};
#pragma endregion

// [ ComputeShader ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region ComputeShader
class ComputeShader {
private:
	bool mIsClosed{ false };

protected:
	ComPtr<ID3D12PipelineState>			mPipelineState{};
	D3D12_COMPUTE_PIPELINE_STATE_DESC	mPipelineStateDesc{};

	ComPtr<ID3DBlob> mCSBlob{};

public:
	ComputeShader() = default;
	virtual ~ComputeShader();

public:
	virtual void Create(bool isClose = true);
	virtual void Set();

protected:
	virtual ComPtr<ID3DBlob> CreateComputeShader();
	void Close();
};

// 수평 흐리기 쉐이더
class HorzBlurShader : public ComputeShader {
public:
	HorzBlurShader()		  = default;
	virtual ~HorzBlurShader() = default;

protected:
	virtual ComPtr<ID3DBlob> CreateComputeShader() override;
};

// 수직 흐리기 쉐이더
class VertBlurShader : public ComputeShader {
public:
	VertBlurShader()		  = default;
	virtual ~VertBlurShader() = default;

protected:
	virtual ComPtr<ID3DBlob> CreateComputeShader() override;
};

// Look-Up-Table 쉐이더
class LUTShader : public ComputeShader {
public:
	LUTShader()			 = default;
	virtual ~LUTShader() = default;

protected:
	virtual ComPtr<ID3DBlob> CreateComputeShader() override;
};




// for rendering UI (2D plane)
class SkinMeshShader : public DeferredShader {
public:
	virtual ComPtr<ID3DBlob> CreateVertexShader() override;
};

#pragma endregion
#pragma endregion
