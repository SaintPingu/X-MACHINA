#pragma once


#pragma region Include
#include "Grid.h"
#pragma endregion

#pragma region Define
#define scene Scene::Inst()
#pragma endregion

#pragma region ClassForwardDecl
class Model;
class MasterModel;
class ModelObjectMesh;

class Shader;
class StaticShader;

class Camera;
class GameObject;
class Material;
class HeightMapTerrain;
class Light;
class Texture;
class SkyBox;
class GraphicsRootSignature;
class DescriptorHeap;
class ObjectInstanceBuffer;
#pragma endregion




#pragma region Class
class Scene {
public:
	enum class ExplosionType { Small = 0, Big };

private:
	/* DirectX */
	sptr<GraphicsRootSignature> mGraphicsRootSignature{};

	/* Model */
	std::unordered_map<std::string, sptr<const MasterModel>> mModels;

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
	sptr<StaticShader> mSmallExpFXShader{};
	sptr<StaticShader> mBigExpFXShader{};

	/* Object */
	sptr<GameObject> mWater{};
	std::vector<sptr<GameObject>> mBackgrounds{};
	std::vector<sptr<GameObject>> mStaticObjects{};
	std::list<sptr<GameObject>> mExplosiveObjects{};
	std::list<sptr<GameObject>> mSpriteEffectObjects{};
	std::vector<sptr<ObjectInstanceBuffer>> mInstanceBuffers;

	/* Player */
	std::vector<sptr<GameObject>> mPlayers{};
	sptr<GameObject> mPlayer{};
	int	mCurrPlayerIndex{};

	/* Map */
	sptr<HeightMapTerrain> mTerrain{};
	BoundingBox mMapBorder{};

	/* Grid */
	std::vector<Grid> mGrids{};
	sptr<ModelObjectMesh> mGridMesh{};
	const float mMaxGridHeight = 300.f;
	float mGridStartPoint{};
	int mGridLength{};
	int mGridCols{};

	/* Descriptor */
	sptr<DescriptorHeap> mDescriptorHeap{};

	/* Others */
	bool mIsRenderBounds{ false };

public:
#pragma region C/Dtor
	Scene();
	~Scene() = default;

	static void Create();
	static void Destroy();
	static void Release();
	static Scene* Inst();
#pragma endregion

#pragma region Getter
	float GetTerrainHeight(float x, float z) const;
	rsptr<const MasterModel> GetModel(const std::string& modelName) const;
	rsptr<GameObject> GetPlayer() const { return mPlayers.front(); }
	rsptr<Material> GetMaterial(const std::string& name) const;
	rsptr<Texture> GetTexture(const std::string& name) const;

	RComPtr<ID3D12RootSignature> GetRootSignature() const;
	UINT GetRootParamIndex(RootParam param) const;
#pragma endregion


public:
#pragma region DirectX
public:
	void ReleaseUploadBuffers();

	void SetGraphicsRoot32BitConstants(RootParam param, const Matrix& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, const Vec4x4& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset);

	void CreateShaderResourceView(RComPtr<ID3D12Resource> resource, DXGI_FORMAT dxgiSrvFormat);
	void CreateShaderResourceView(Texture* texture, UINT descriptorHeapIndex);

private:
	void CreateGraphicsRootSignature();
	void CreateCbvSrvDescriptorHeaps(int cbvCount, int srvCount);

	void CreateShaderVars();
	void UpdateShaderVars();
	void ReleaseShaderVars();

#pragma endregion


#pragma region Build
public:
	void BuildObjects();
	void ReleaseObjects();

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

	/* Load */
	void LoadTextures();
	void LoadSceneObjectsFromFile(const std::string& fileName);
	void LoadGameObjects(FILE* file);
	void LoadModels();

	/* Other */
	void InitObjectByTag(const void* tag, sptr<GameObject> object);

#pragma endregion


#pragma region Render
public:
	void OnPrepareRender();
	void Render();

private:
	void RenderGridObjects(std::set<GameObject*>& opaqueObjects, std::set<GameObject*>& transparentObjects, std::set<GameObject*>& billboardObjects);
	void RenderBackgrounds();
	void RenderBullets();
	void RenderInstanceObjects();
	void RenderFXObjects();
	void RenderBillboards(const std::set<GameObject*>& billboards);

	void RenderTerrain();

	void RenderTransparentObjects(const std::set<GameObject*>& transparentObjects);
	void RenderSkyBox();
	bool RenderBounds(const std::set<GameObject*>& opaqueObjects);
	void RenderObjectBounds(const std::set<GameObject*>& opaqueObjects);
	void RenderGridBounds();
#pragma endregion


#pragma region Update
public:
	void Start();
	void Update();
	void Animate();

private:
	void CheckCollisions();
	void UpdatePlayerGrid();

	void UpdateObjects();
	void UpdateObject(GameObject* object);
	void UpdateFXObjects();

	void UpdateSprites();
	void UpdateLights();
	void UpdateCamera();

#pragma endregion

public:
	void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

	void ToggleDrawBoundings();

	void CreateExplosion(ExplosionType explosionType, const Vec3& pos);
	void BlowAllExplosiveObjects();

	void ChangeToNextPlayer();
	void ChangeToPrevPlayer();

	void UpdateObjectGrid(GameObject* object, bool isCheckAdj = true);
	void RemoveObjectFromGrid(GameObject* object);

	void ProcessObjects(std::function<void(sptr<GameObject>)> processFunc);

private:
	void CreateSpriteEffect(Vec3 pos, float speed, float scale = 1);
	void CreateSmallExpFX(Vec3 pos);
	void CreateBigExpFX(Vec3 pos);

	int GetGridIndexFromPos(Vec3 pos);
	void SetObjectGridIndex(rsptr<GameObject> object, int gridIndex);
	bool IsGridOutOfRange(int index) { return index < 0 || index >= mGrids.size(); }

	void DeleteExplodedObjects();
};
#pragma endregion
