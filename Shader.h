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
// Color를 가지는 객체의 인스턴싱 StructuredBuffer
struct SB_ColorInst {
	Vec4x4	LocalTransform{};
	Vec4	Color{};
};
#pragma endregion


#pragma region Class
/* Shader의 Render() 함수를 모두 제거하고 별도로 관리하도록 재구현할 예정 */
/* Shader는 주로 PipelineState를 설정하는 용도로 사용하기 위함		      */


// Pipeline State 객체들을 가지는 클래스
class Shader {
private:
	bool mIsClosed{ false };	// PipelineState생성을 종료하고 불필요 메모리를 해제했는가?

protected:
	std::vector<ComPtr<ID3D12PipelineState>>	mPipelineStates{};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC			mPipelineStateDesc{};	// 마지막으로 설정된 PipelineStateDesc

	ComPtr<ID3DBlob> mVSBlob{};
	ComPtr<ID3DBlob> mPSBlob{};

public:
	Shader() = default;
	virtual ~Shader();

public:
	// [mPipelineStateDesc]를 설정하고 PipelineState를 생성한다.
	virtual void Create(DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT, bool isClose = true);

	// [pipelineStateIndex]의 PipelineState를 설정하고 쉐이더 변수를 업데이트한다.
	// 렌더링 전 반드시 이 함수를 호출해야한다.
	virtual void Set(int pipelineStateIndex = 0);

protected:
	virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveType() const { return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; };

	// [pipelineStateIndex] 의 PipelineState를 설정한다.
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

	// PipelineState 생성을 중단하고, 생성 과정에 발생한 불필요한 메모리를 해제한다.
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
	// mObjects를 그룹으로 나눈다.
	// 활성화된 그룹
	// <시작 인덱스(begin), 현재 수명 시간(lifeTime)>
	std::unordered_map<size_t, float> mActiveGroups{};
	size_t mGroupSize{};
	size_t mCountPerGroup{};

	std::vector<sptr<Script_Fragment>> mObjectScripts{};

private:
	float mMaxLifeTime{};

	std::vector<size_t> mTimeOvers{};	// [mActiveGroups]에서 시간을 초과한 객체의 <시작 인덱스(bgein)>

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
// 이펙트의 형식이 미리 정의된 쉐이더
// Create 시 이펙트 객체 pool을 자동으로 생성한다.
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
	std::list<sptr<GameObject>> mBuffer{};	// 활성화된 객체 리스트 (배열로 변경 필요)

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

	// [pos] 위치에 생성하고 [dir, up]에 따라 look 방향을 결정하고, look 방향으로 [speed]의 속도로 이동하도록 한다.
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
	// texture resource를 생성하고 이에 대한 SRV와 RTV를 생성한다
	virtual void CreateResourcesAndRtvsSrvs(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

	// 각 RTV의 handle을 OutputMerger에 Set한다.
	virtual void OnPrepareRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle);
	// 렌더링 후 리소스의 상태를 전이한다. (ResourceBarrier)
	virtual void OnPostRenderTarget();

	// 2D plane을 렌더링한다.
	virtual void Render();

protected:
	virtual void UpdateShaderVars() override;

private:
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

	// texture resource를 생성한다 (ID3D12Resource)
	void CreateTextureResources();
	// resource의 SRV Descriptor를 생성한다. (ID3D12Device::CreateShaderResourceView)
	void CreateSrvs();
	// resource의 RTV Descriptor를 생성한다. (ID3D12Device::CreateRenderTargetView)
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
