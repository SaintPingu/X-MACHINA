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
struct SB_ColorInst {
	Vec4x4	LocalTransform{};
	Vec4	Color{};
};
#pragma endregion


#pragma region Class
class Shader {
private:
	bool mIsClosed{ true };

protected:
	std::vector<ComPtr<ID3D12PipelineState>>	mPipelineStates;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC			mPipelineStateDesc{};

	ComPtr<ID3DBlob> mVSBlob{};
	ComPtr<ID3DBlob> mPSBlob{};

public:
	Shader() = default;
	virtual ~Shader();

public:
	virtual void Create(DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT, bool isClose = true);

	virtual void CreateShaderVars()  {};
	virtual void UpdateShaderVars()  {};
	virtual void ReleaseShaderVars() {};

	virtual void Render(int pipelineStateIndex = 0);

protected:
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType() const { return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; };

	void SetPipelineState(int pipelineStateIndex = 0);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	D3D12_SHADER_BYTECODE CompileShaderFile(const std::wstring& fileName, LPCSTR shaderName, LPCSTR shaderProfile, ComPtr<ID3DBlob>& shaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFile(const std::wstring& fileName, ComPtr<ID3DBlob>& shaderBlob);

	void Close();
};





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





class InstancingShader : public Shader {
private:
	using InstBuff = SB_ColorInst;

protected:
	std::vector<sptr<GameObject>>	mObjects{};
	ComPtr<ID3D12Resource>			mInstBuff{};
	InstBuff* mMappedObjects{};
	sptr<const Mesh>				mMesh;

public:
	InstancingShader() = default;
	virtual ~InstancingShader();

	std::vector<sptr<GameObject>>& GetObjects() { return mObjects; }

	void SetColor(const Vec3& color);

public:
	virtual void Start();
	virtual void Update();
	virtual void Render();

	void BuildObjects(size_t instanceCount, rsptr<const Mesh> mesh);

protected:
	void SetSRV();

	virtual void CreateShaderVars() override;
	virtual void UpdateShaderVars() override;
	virtual void ReleaseShaderVars() override;

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

	template<class Pred>
	void DoAllObjects(Pred pred)
	{
		for (const auto& object : mObjects) {
			pred(object);
		}
	}
};






class EffectShader : public InstancingShader {

protected:
	// mObjects를 그룹으로 나눈다.
	// 활성화된 그룹
	// <begin, duration>
	std::unordered_map<size_t, float> mActiveGroups{};
	size_t mGroupSize{};
	size_t mCountPerGroup{};

	std::vector<sptr<Script_Fragment>> mObjectScripts{};

private:
	float mMaxDuration{};

	// 시간 초과 객체 관리
	std::vector<size_t> mTimeOvers{};

public:
	EffectShader()          = default;
	virtual ~EffectShader() = default;

	void SetDuration(float duration) { mMaxDuration = duration; }
	void SetColor(size_t i, const Vec3& color);

public:
	virtual void Update() override;
	virtual void Render() override;

	void BuildObjects(size_t groupCount, size_t countPerGroup, rsptr<const ModelObjectMesh> mesh);

	void SetActive(const Vec3& pos);

protected:
	virtual void UpdateShaderVars() override;
};





class TexturedEffectShader : public EffectShader {
private:
	sptr<Material> mMaterial;
	
public:
	TexturedEffectShader()          = default;
	virtual ~TexturedEffectShader() = default;

public:
	void SetMaterial(rsptr<Material> material) { mMaterial = material; }

	virtual void Render() override;

protected:
	virtual void UpdateShaderVars() override;

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};




#pragma region StaticShader
class StaticShader abstract : public TexturedEffectShader {
private:
	virtual void BuildObjects() abstract;

public:
	virtual void Create();
};

class SmallExpEffectShader : public StaticShader {
private:
	virtual void BuildObjects() override;
};

class BigExpEffectShader : public StaticShader {
private:
	virtual void BuildObjects() override;
};
#pragma endregion






class BulletShader : public InstancingShader {
protected:
	std::vector<sptr<Script_Bullet>> mObjectScripts{};

private:
	std::list<sptr<GameObject>> mBuffer{};

public:
	BulletShader()          = default;
	virtual ~BulletShader() = default;

	const std::vector<sptr<GameObject>>& GetObjects() const { return mObjects; }
	void BuildObjects(size_t bufferSize, rsptr<const MasterModel> model, const Object* owner);
	void SetLifeTime(float bulletLifeTime);
	void SetDamage(float damage);

public:
	virtual void Start() override;
	virtual void Update() override;
	virtual void Render() override;

	void FireBullet(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed);

	const std::list<sptr<GameObject>>* GetBullets() const { return &mBuffer; }

protected:
	virtual void UpdateShaderVars() override;
};





class TexturedShader : public Shader {
public:
	TexturedShader()          = default;
	virtual ~TexturedShader() = default;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

};





class ObjectInstancingShader : public TexturedShader {
public:
	ObjectInstancingShader()          = default;
	virtual ~ObjectInstancingShader() = default;

protected:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
};





class TransparentShader : public TexturedShader {
public:
	TransparentShader()          = default;
	virtual ~TransparentShader() = default;

protected:
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;
	virtual D3D12_BLEND_DESC CreateBlendState() override;
};





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
	virtual void CreateResourcesAndRtvsSrvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

	virtual void OnPrepareRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle);
	virtual void OnPostRenderTarget();

	virtual void Render();

private:
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

	void CreateTextureResources();
	void CreateSrvs();
	void CreateRtvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
};





class TextureToScreenShader : public PostProcessingShader {
public:
	TextureToScreenShader()          = default;
	virtual ~TextureToScreenShader() = default;

protected:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
};




#pragma region BillboardShader
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

class SpriteShader : public BillboardShader {
public:
	SpriteShader()          = default;
	virtual ~SpriteShader() = default;

protected:
	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
};
#pragma endregion




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
