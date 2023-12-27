//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once
#include "Grid.h"

class CMasterModel;
class CModelObjectMesh;

class CModel;
class Camera;
class CCameraObject;
class CGameObject;
class CShader;
class CMirrorShader;

class CMaterial;

class CInstancingShader;
class CEffectShader;
class CTexturedEffectShader;
class CModelObjectsShader;
class CTexturedShader;
class CStaticShader;

class CHeightMapTerrain;

class CLight;
class CMainCamera;

class CTexture;
class CSkyBox;

class CGraphicsRootSignature;
class CDescriptorHeap;

struct MATERIAL
{
	Vec4 mAmbient{};
	Vec4 mDiffuse{};
	Vec4 mSpecular{};
	Vec4 mEmissive{};
};

class CObjectInstanceBuffer;
class CScene
{
public:
	enum class ExplosionType { Small = 0, Big };

private:
	/* DirectX */
	sptr<CGraphicsRootSignature> mGraphicsRootSignature{};

	/* Model */
	std::unordered_map<std::string, sptr<const CMasterModel>> mModels;
	std::vector<sptr<CObjectInstanceBuffer>> mInstanceBuffers;

	/* Light */
	uptr<CLight> mLight;

	/* Textures */
	std::unordered_map<std::string, sptr<CMaterial>> mMaterialMap{};

	/* SkyBox */
	sptr<CSkyBox> mSkyBox{};

	/* Shader */
	sptr<CShader> mGlobalShader{};
	sptr<CShader> mBoundingShader{};
	sptr<CShader> mWaterShader{};
	sptr<CShader> mBillboardShader{};
	sptr<CShader> mSpriteShader{};
	sptr<CShader> mInstancingShader{};
	sptr<CShader> mTransparentShader{};
	sptr<CStaticShader> mSmallExpFXShader{};
	sptr<CStaticShader> mBigExpFXShader{};
	sptr<CMirrorShader> mMirrorShader{};

	/* Camera*/
	sptr<CMainCamera> mMainCamera{};

	/* Object */
	sptr<CGameObject> mWater{};
	std::vector<sptr<CGameObject>> mBackgrounds{};
	std::vector<sptr<CGameObject>> mStaticObjects{};
	std::list<sptr<CGameObject>> mExplosiveObjects{};
	std::list<sptr<CGameObject>> mSpriteEffectObjects{};

	/* Player */
	std::vector<sptr<CGameObject>> mPlayers{};
	sptr<CGameObject> mPlayer{};
	int	mCrntPlayerIndex{};

	/* Map */
	sptr<CHeightMapTerrain> mTerrain{};
	BoundingBox mMapBorder{};

	/* Grid */
	std::vector<CGrid> mGrids{};
	sptr<CModelObjectMesh> mGridMesh{};
	const float mMaxGridHeight = 300.0f;
	float mGridStartPoint{};
	int mGridLength{};
	int mGridCols{};

	/* Descriptor */
	sptr<CDescriptorHeap> mDescriptorHeap{};

	/* Others */
	bool mIsRenderBounds{ false };

public:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Constructor ] /////

	CScene();
	~CScene();

	static void Create();
	static void Destroy();
	static CScene* Inst();


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Getter ] /////

	float GetTerrainHeight(float x, float z) const;
	rsptr<const CMasterModel> GetModel(const std::string& modelName);
	rsptr<CGameObject> GetPlayer() const { return mPlayers.front(); }
	rsptr<CMaterial> GetMaterial(const std::string& name) { assert(mMaterialMap.contains(name)); return mMaterialMap[name]; }
	rsptr<CTexture> GetTexture(const std::string& name);
	rsptr<Camera> GetMainCamera() const;
	sptr<CCameraObject> GetCameraObject() const;
	RComPtr<ID3D12RootSignature> GetRootSignature() const;
	//const LIGHT* GetLightModel(const std::string& modelName);

	UINT GetRootParamIndex(RootParam param);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Setter ] /////
	void SetGlobalShader() const;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Others ] /////
	
	/* DirectX */
	void CreateGraphicsRootSignature();

	void CreateShaderVariables();
	void UpdateShaderVariables();
	void ReleaseShaderVariables();
	void ReleaseUploadBuffers();

public:
	void SetGraphicsRoot32BitConstants(RootParam param, const Matrix& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, const Vec4x4& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset);



	/* Build */
private:
	/* Shader */
	void BuildShaders();

	void BuildGlobalShader();
	void BuildBoundingShader();
	void BuildSmallExpFXShader();
	void BuildBigExpFXShader();
	void BuildBillboardShader();

	/* Object */
	void BuildPlayers();
	void BuildTerrain();

	/* Grid */
	void BuildGrid();
	void BuildGridObjects();

	/* Camera */
	void BuildCamera();

	/* Load */
	void LoadModels();
private:
	void InitObjectByTag(const void* tag, sptr<CGameObject> object);

public:
	void LoadGameObjects(FILE* file);
	void LoadSceneObjectsFromFile(const std::string& fileName);
	void CreateCbvSrvDescriptorHeaps(int cbvCount, int srvCount);
	void LoadTextures();

public:
	void BuildObjects();
	void ReleaseObjects();

	void CreateShaderResourceView(RComPtr<ID3D12Resource> resource, DXGI_FORMAT dxgiSrvFormat);
	void CreateShaderResourceView(CTexture* texture, UINT descriptorHeapIndex);



	/* Render */
private:
	void RenderTerrain(bool isMirror = false);

	void RenderGridObjects(std::set<CGameObject*>& renderObjects, std::set<CGameObject*>& transparentObjects, std::set<CGameObject*>& billboardObjects);
	void RenderInstanceObjects();
	void RenderBackgrounds();
	void RenderBullets();

	void RenderBounds(const std::set<CGameObject*>& renderObjects);
	void RenderGridBounds();
	void RenderBillboards(const std::set<CGameObject*>& billboards);

public:
	void OnPrepareRender();
	void Render();
	void RenderMirrorObjects(const Vec4& mirrorPlane);



	/* Update (per frame) */
private:
	void UpdateLights();
	void UpdateObjects();
	void UpdateCamera();
	void AnimateObjects();
	void CheckCollisions();


	void UpdatePlayerGrid();
	void UpdateObject(CGameObject* object);

public:
	void Start();
	void Update();


	/* Others */
private:
	void CreateSpriteEffect(Vec3 pos, float speed, float scale = 1);
	void CreateSmallExpFX(Vec3 pos);
	void CreateBigExpFX(Vec3 pos);

	int GetGridIndexFromPos(Vec3 pos);
	void SetObjectGridIndex(rsptr<CGameObject> object, int gridIndex);
	bool IsGridOutOfRange(int index) { return index < 0 || index >= mGrids.size(); }

	void DeleteExplodedObjects();

public:
	void ProcessInput(HWND hWnd, POINT oldCursorPos);
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void ToggleDrawBoundings();

	void CreateExplosion(ExplosionType explosionType, const Vec3& pos);
	void BlowAllExplosiveObjects();

	void ChangeToNextPlayer();
	void ChangeToPrevPlayer();

	void UpdateObjectGrid(CGameObject* object, bool isCheckAdj = true);
	void RemoveObjectFromGrid(CGameObject* object);

	void ProcessObjects(std::function<void(sptr<CGameObject>)> processFunc);

	void RenderMirror();
	const XMMATRIX& GetReflect() const;
	bool IsRenderReflectObject();
};

#define crntScene CScene::Inst()
#define mainCamera crntScene->GetMainCamera()