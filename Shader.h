#pragma once

#pragma region ClassForwardDecl
class ModelObjectMesh;
class Mesh;
class GameObject;
class MasterModel;
class Object;
class Material;
class Texture;

class Script_Fragment;
class Script_Bullet;
#pragma endregion


#pragma region Struct
// Color�� ������ ��ü�� �ν��Ͻ� StructuredBuffer
struct SB_ColorInst {
	Vec4x4	LocalTransform{};
	Vec4	Color{};
};
#pragma endregion


#pragma region Class
/* Shader�� Render() �Լ��� ��� �����ϰ� ������ �����ϵ��� �籸���� ���� */
/* Shader�� �ַ� PipelineState�� �����ϴ� �뵵�� ����ϱ� ����		      */


// Pipeline State ��ü���� ������ Ŭ����
class Shader {
private:
	bool mIsClosed{ false };	// PipelineState������ �����ϰ� ���ʿ� �޸𸮸� �����ߴ°�?

protected:
	std::vector<ComPtr<ID3D12PipelineState>>	mPipelineStates{};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC			mPipelineStateDesc{};	// ���������� ������ PipelineStateDesc

	ComPtr<ID3DBlob> mVSBlob{};
	ComPtr<ID3DBlob> mPSBlob{};

public:
	Shader() = default;
	virtual ~Shader();

public:
	// [mPipelineStateDesc]�� �����ϰ� PipelineState�� �����Ѵ�.
	virtual void Create(DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT, bool isClose = true);

	// [pipelineStateIndex]�� PipelineState�� �����ϰ� ���̴� ������ ������Ʈ�Ѵ�.
	// ������ �� �ݵ�� �� �Լ��� ȣ���ؾ��Ѵ�.
	virtual void Set(int pipelineStateIndex = 0);

protected:
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType() const { return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; };

	// [pipelineStateIndex] �� PipelineState�� �����Ѵ�.
	void SetPipelineState(int pipelineStateIndex = 0);

	virtual void CreateShaderVars() {};
	virtual void UpdateShaderVars() {};
	virtual void ReleaseShaderVars() {};

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	D3D12_SHADER_BYTECODE CompileShaderFile(const std::wstring& fileName, LPCSTR shaderName, LPCSTR shaderProfile, ComPtr<ID3DBlob>& shaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFile(const std::wstring& fileName, ComPtr<ID3DBlob>& shaderBlob);

	// PipelineState ������ �ߴ��ϰ�, ���� ������ �߻��� ���ʿ��� �޸𸮸� �����Ѵ�.
	void Close();
};





// for rendering wireframe
class WireShader : public Shader {
public:
	WireShader()          = default;
	virtual ~WireShader() = default;

protected:
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType() const override { return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};





// for rendering instancing GameObjects
class InstShader : public Shader {
private:
	using InstBuff = SB_ColorInst;

protected:
	std::vector<sptr<GameObject>>	mObjects{};			// all objects
	ComPtr<ID3D12Resource>			mSB_Inst{};			// StructuredBuffer for instance
	InstBuff*						mSBMap_Inst{};		// mapped StructuredBuffer
	sptr<const Mesh>				mMesh;				// has only one model mesh

public:
	InstShader() = default;
	virtual ~InstShader();

	// set color for all objects
	void SetColor(const Vec3& color);

public:
	virtual void Start();
	virtual void Update();
	virtual void Render();

	// create object pool as mush as [instCnt] and set model mesh to [mesh]
	void BuildObjects(size_t instCnt, rsptr<const Mesh> mesh);

protected:
	// set SRV that instancing StructuredBuffer
	void SetShaderResourceView();

	virtual void CreateShaderVars() override;
	virtual void UpdateShaderVars() override;
	virtual void ReleaseShaderVars() override;

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};






// for rendering 3d effect GameObjects
class EffectShader : public InstShader {

protected:
	// mObjects�� �׷����� ������.
	// Ȱ��ȭ�� �׷�
	// <���� �ε���(begin), ���� ���� �ð�(lifeTime)>
	std::unordered_map<size_t, float> mActiveGroups{};
	size_t mGroupSize{};
	size_t mCountPerGroup{};

	std::vector<sptr<Script_Fragment>> mObjectScripts{};

private:
	float mMaxLifeTime{};

	std::vector<size_t> mTimeOvers{};	// [mActiveGroups]���� �ð��� �ʰ��� ��ü�� <���� �ε���(bgein)>

public:
	EffectShader()          = default;
	virtual ~EffectShader() = default;

	void SetLifeTime(float lifeTime) { mMaxLifeTime = lifeTime; }

public:
	virtual void Update() override;
	virtual void Render() override;

	void BuildObjects(size_t groupCount, size_t countPerGroup, rsptr<const ModelObjectMesh> mesh);

	// create effect at [pos]
	void SetActive(const Vec3& pos);

protected:
	virtual void UpdateShaderVars() override;
};






// for rendering 3d effect GameObjects that has texture
class TexturedEffectShader : public EffectShader {
private:
	sptr<Material> mMaterial;	// has only one model material
	
public:
	TexturedEffectShader();
	virtual ~TexturedEffectShader() = default;

	void SetTexture(rsptr<Texture> texture);

public:
	virtual void Render() override;

protected:
	virtual void UpdateShaderVars() override;

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};




#pragma region StaticShader
// ����Ʈ�� ������ �̸� ���ǵ� ���̴�
// Create �� ����Ʈ ��ü pool�� �ڵ����� �����Ѵ�.
class StaticShader abstract : public TexturedEffectShader {
private:
	virtual void BuildObjects() abstract;

public:
	virtual void Create();
};

// small explosion
class SmallExpEffectShader : public StaticShader {
private:
	virtual void BuildObjects() override;
};

// big explosion
class BigExpEffectShader : public StaticShader {
private:
	virtual void BuildObjects() override;
};
#pragma endregion






// for bullet GameObjects
class BulletShader : public InstShader {
protected:
	std::vector<sptr<Script_Bullet>> mObjectScripts{};

private:
	std::list<sptr<GameObject>> mBuffer{};	// Ȱ��ȭ�� ��ü ����Ʈ (�迭�� ���� �ʿ�)

public:
	BulletShader()          = default;
	virtual ~BulletShader() = default;

	// create object buffer(pool). set [model], [owner]
	void BuildObjects(rsptr<const MasterModel> model, const Object* owner);
	void SetLifeTime(float bulletLifeTime);
	void SetDamage(float damage);

public:
	virtual void Start() override;
	virtual void Update() override;
	virtual void Render() override;

	// [pos] ��ġ�� �����ϰ� [dir, up]�� ���� look ������ �����ϰ�, look �������� [speed]�� �ӵ��� �̵��ϵ��� �Ѵ�.
	void FireBullet(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed);

protected:
	virtual void UpdateShaderVars() override;
};





// for rendering GameObjects that has texture
class TexturedShader : public Shader {
public:
	TexturedShader()          = default;
	virtual ~TexturedShader() = default;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

};





// for rendering instancing GameObjects that has texture
class ObjectInstShader : public TexturedShader {
public:
	ObjectInstShader()          = default;
	virtual ~ObjectInstShader() = default;

protected:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
};





// for rendering transparent GameObjects
class TransparentShader : public TexturedShader {
public:
	TransparentShader()          = default;
	virtual ~TransparentShader() = default;

protected:
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;
	virtual D3D12_BLEND_DESC CreateBlendState() override;
};





// for rendering Terrain
class TerrainShader : public Shader {
public:
	TerrainShader()          = default;
	virtual ~TerrainShader() = default;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

};





// for rendering SkyBox
class SkyBoxShader : public Shader {
public:
	SkyBoxShader()          = default;
	virtual ~SkyBoxShader() = default;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};





// for rendering instancing GameObjects
class WaterShader : public TexturedShader {
public:
	WaterShader()          = default;
	virtual ~WaterShader() = default;

protected:
	virtual D3D12_BLEND_DESC CreateBlendState() override;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};





// for post processing
class PostProcessingShader : public Shader {
protected:
	std::vector<sptr<Texture>> mTextures{};

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> mRtvHandles{};

private:
	UINT mRtvCnt{};
	const DXGI_FORMAT* mRtvFormats{};

public:
	PostProcessingShader();
	virtual ~PostProcessingShader() = default;

public:
	// texture resource�� �����ϰ� �̿� ���� SRV�� RTV�� �����Ѵ�
	virtual void CreateResourcesAndRtvsSrvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

	// �� RTV�� handle�� OutputMerger�� Set�Ѵ�.
	virtual void OnPrepareRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle);
	// ������ �� ���ҽ��� ���¸� �����Ѵ�. (ResourceBarrier)
	virtual void OnPostRenderTarget();

	// 2D plane�� �������Ѵ�.
	virtual void Render();

protected:
	virtual void UpdateShaderVars() override;

private:
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

	// texture resource�� �����Ѵ� (ID3D12Resource)
	void CreateTextureResources();
	// resource�� SRV Descriptor�� �����Ѵ�. (ID3D12Device::CreateShaderResourceView)
	void CreateSrvs();
	// resource�� RTV Descriptor�� �����Ѵ�. (ID3D12Device::CreateRenderTargetView)
	void CreateRtvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
};





// for rendering 2D texture to screen
class TextureToScreenShader : public PostProcessingShader {
public:
	TextureToScreenShader()          = default;
	virtual ~TextureToScreenShader() = default;

protected:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};





#pragma region BillboardShader
// for rendering billboard GameObjects
class BillboardShader : public TexturedShader {
public:
	BillboardShader()          = default;
	virtual ~BillboardShader() = default;

protected:
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;
	virtual D3D12_BLEND_DESC CreateBlendState() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};

// for rendering sprite GameObjects
class SpriteShader : public BillboardShader {
public:
	SpriteShader()          = default;
	virtual ~SpriteShader() = default;

protected:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
};
#pragma endregion





// for rendering UI (2D plane)
class CanvasShader : public TexturedShader {
public:
	CanvasShader()          = default;
	virtual ~CanvasShader() = default;

public:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;
	virtual D3D12_BLEND_DESC CreateBlendState() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};
#pragma endregion
