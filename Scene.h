//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once
#include "Grid.h"

class MasterModel;
class ModelObjectMesh;

class Model;
class Camera;
class CameraObject;
class GameObject;
class Shader;

class Material;

class InstancingShader;
class EffectShader;
class TexturedEffectShader;
class ModelObjectsShader;
class TexturedShader;
class StatiShader;

class HeightMapTerrain;

class Light;
class MainCamera;

class Texture;
class SkyBox;

class GraphicsRootSignature;
class DescriptorHeap;

struct MATERIAL
{
	Vec4 mAmbient{};
	Vec4 mDiffuse{};
	Vec4 mSpecular{};
	Vec4 mEmissive{};
};

class ObjectInstanceBuffer;
class Scene
{
public:
	enum class ExplosionType { Small = 0, Big };

private:
	/* DirectX */
	sptr<GraphicsRootSignature> mGraphicsRootSignature{};

	/* Model */
	std::unordered_map<std::string, sptr<const MasterModel>> mModels;
	std::vector<sptr<ObjectInstanceBuffer>> mInstanceBuffers;

	/* Light */
	uptr<Light> mLight;

	/* Textures */
	std::unordered_map<std::string, sptr<Material>> mMaterialMap{};

	/* SkyBox */
	sptr<SkyBox> mSkyBox{};

	/* Shader */
	sptr<Shader> mGlobalShader{};
	sptr<Shader> mBoundingShader{};
	sptr<Shader> mWaterShader{};
	sptr<Shader> mBillboardShader{};
	sptr<Shader> mSpriteShader{};
	sptr<Shader> mInstancingShader{};
	sptr<Shader> mTransparentShader{};
	sptr<StatiShader> mSmallExpFXShader{};
	sptr<StatiShader> mBigExpFXShader{};

	/* Camera*/
	sptr<MainCamera> mMainCamera{};

	/* Object */
	sptr<GameObject> mWater{};
	std::vector<sptr<GameObject>> mBackgrounds{};
	std::vector<sptr<GameObject>> mStatiObjects{};
	std::list<sptr<GameObject>> mExplosiveObjects{};
	std::list<sptr<GameObject>> mSpriteEffectObjects{};

	/* Player */
	std::vector<sptr<GameObject>> mPlayers{};
	sptr<GameObject> mPlayer{};
	int	mCrntPlayerIndex{};

	/* Map */
	sptr<HeightMapTerrain> mTerrain{};
	BoundingBox mMapBorder{};

	/* Grid */
	std::vector<Grid> mGrids{};
	sptr<ModelObjectMesh> mGridMesh{};
	const float mMaxGridHeight = 300.0f;
	float mGridStartPoint{};
	int mGridLength{};
	int mGridCols{};

	/* Descriptor */
	sptr<DescriptorHeap> mDescriptorHeap{};

	/* Others */
	bool mIsRenderBounds{ false };

public:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Constructor ] /////

	Scene();
	~Scene();

	static void Create();
	static void Destroy();
	static Scene* Inst();


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Getter ] /////

	float GetTerrainHeight(float x, float z) const;
	rsptr<const MasterModel> GetModel(const std::string& modelName);
	rsptr<GameObject> GetPlayer() const { return mPlayers.front(); }
	rsptr<Material> GetMaterial(const std::string& name) { assert(mMaterialMap.contains(name)); return mMaterialMap[name]; }
	rsptr<Texture> GetTexture(const std::string& name);
	rsptr<Camera> GetMainCamera() const;
	sptr<CameraObject> GetCameraObject() const;
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
	void InitObjectByTag(const void* tag, sptr<GameObject> object);

public:
	void LoadGameObjects(FILE* file);
	void LoadSceneObjectsFromFile(const std::string& fileName);
	void CreateCbvSrvDescriptorHeaps(int cbvCount, int srvCount);
	void LoadTextures();

public:
	void BuildObjects();
	void ReleaseObjects();

	void CreateShaderResourceView(RComPtr<ID3D12Resource> resource, DXGI_FORMAT dxgiSrvFormat);
	void CreateShaderResourceView(Texture* texture, UINT descriptorHeapIndex);



	/* Render */
private:
	void RenderTerrain();

	void RenderGridObjects(std::set<GameObject*>& renderObjects, std::set<GameObject*>& transparentObjects, std::set<GameObject*>& billboardObjects);
	void RenderInstanceObjects();
	void RenderBackgrounds();
	void RenderBullets();

	void RenderBounds(const std::set<GameObject*>& renderObjects);
	void RenderGridBounds();
	void RenderBillboards(const std::set<GameObject*>& billboards);

public:
	void OnPrepareRender();
	void Render();



	/* Update (per frame) */
private:
	void UpdateLights();
	void UpdateObjects();
	void UpdateCamera();
	void AnimateObjects();
	void CheckCollisions();


	void UpdatePlayerGrid();
	void UpdateObject(GameObject* object);

public:
	void Start();
	void Update();


	/* Others */
private:
	void CreateSpriteEffect(Vec3 pos, float speed, float scale = 1);
	void CreateSmallExpFX(Vec3 pos);
	void CreateBigExpFX(Vec3 pos);

	int GetGridIndexFromPos(Vec3 pos);
	void SetObjectGridIndex(rsptr<GameObject> object, int gridIndex);
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

	void UpdateObjectGrid(GameObject* object, bool isCheckAdj = true);
	void RemoveObjectFromGrid(GameObject* object);

	void ProcessObjects(std::function<void(sptr<GameObject>)> processFunc);
};

#define crntScene Scene::Inst()
#define mainCamera crntScene->GetMainCamera()