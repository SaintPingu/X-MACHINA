#pragma once


#pragma region Include
#include "Grid.h"
#pragma endregion


#pragma region Define
#define GAME_MGR Scene::I->GetGameManager()
#pragma endregion


#pragma region ClassForwardDecl
class Camera;
class Object;
class GameObject;
class GridObject;
class InstObject;
class Terrain;
class Light;
class SkyBox;
class ObjectPool;
class TestCube;
class MasterModel;
class ObjectTag;
#pragma endregion


#pragma region EnumClass
enum class RenderType : UINT8 {
	Shadow,
	Forward,
	Deferred,
	CustomDepth,
	DynamicEnvironmentMapping,
};
#pragma endregion


#pragma region Class
class Scene : public Singleton<Scene> {
	friend Singleton;

public:
	enum class FXType {
		SmallExplosion = 0,
		BigExplosion
	};

private:
	/* Light */
	sptr<Light> mLight{};

	/* SkyBox */
	sptr<SkyBox> mSkyBox{};

	/* Object */
	sptr<Object>					mGameManager{};
	std::vector<sptr<GameObject>>	mEnvironments{};
	std::vector<sptr<GridObject>>	mStaticObjects{};
	std::vector<sptr<GridObject>>	mDynamicObjects{};
	std::vector<sptr<ObjectPool>>	mObjectPools{};
	std::vector<sptr<GridObject>>	mDynamicObjectBuffer{};		// 추가(Instantiate) 대기 버퍼
	std::vector<sptr<Object>>		mScriptObjects{};			// Unity Scene에서 스크립트를 가지고 있는 객체들
	std::set<size_t>				mDestroyObjects{};

	std::set<sptr<GridObject>>		mDissolveObjects{};
	std::set<GridObject*>			mRenderedObjects{};
	std::set<GridObject*>			mSkinMeshObjects{};
	std::set<GridObject*>			mGridObjects{};
	
	std::map<ObjectTag, std::set<GridObject*>> mObjectsByShader;

	/* Map */
	sptr<Terrain>		mTerrain{};
	BoundingBox			mMapBorder{};			// max scene range	(grid will be generated within this border)

	/* Grid */
	std::vector<sptr<Grid>>	mGrids{};				// all scene grids
	std::vector<sptr<Grid>>	mSurroundGrids{};		// around player grids

	float					mGridStartPoint{};		// leftmost coord of the entire grid
	int						mGridXLength{};			// length of x for one grid
	int						mGridZLength{};			// length of z for one grid
	int						mGridXCount{};			// number of columns(x) in the grid
	int						mGridZCount{};			// number of rows(z)    in the grid

	/* Others */
	bool mIsRenderBounds = false;

	std::vector<Vec3>	mOpenList{};
	std::vector<Vec3>	mClosedList{};

private:
#pragma region C/Dtor
	Scene();
	virtual ~Scene() = default;

public:
	void Release();

private:
#pragma endregion

public:
#pragma region Getter
	float GetTerrainHeight(float x, float z) const;
	std::vector<sptr<GameObject>> GetAllObjects() const;
	std::set<GridObject*> GetRenderedObjects() const { return mRenderedObjects; }
	rsptr<Object> GetGameManager() const { return mGameManager; }

	rsptr<Grid> GetGridFromPos(const Vec3& pos);
	int GetGridIndexFromPos(Vec3 pos) const;

	Pos GetTileUniqueIndexFromPos(const Vec3& pos) const;
	Vec3 GetTilePosFromUniqueIndex(const Pos& index) const;

	Tile GetTileFromUniqueIndex(const Pos& index) const;
	void SetTileFromUniqueIndex(const Pos& index, Tile tile);
	Tile GetTileFromPos(const Vec3& index) const;

	std::vector<Vec3>& GetOpenList() { return mOpenList; }
	std::vector<Vec3>& GetClosedList() { return mClosedList; }

#pragma endregion

#pragma region DirectX
public:
	void ReleaseUploadBuffers();
	void UpdateAbilityCB(int& idx, const AbilityConstants& value);
	void SetAbilityCB(int idx) const;

private:
	void UpdateShaderVars();
	void UpdateMainPassCB();
	void UpdateShadowPassCB();
	void UpdateSsaoCB();
	void UpdateMaterialBuffer();
#pragma endregion

#pragma region Build
public:
	void BuildObjects();
	void ReleaseObjects();

	// ScriptExporter 정보를 로드한다.
	void LoadScriptExporter(std::ifstream& file, rsptr<Object> object);

private:
	/* Object */
	void BuildTerrain();

	/* Grid */
	// generate grids
	void BuildGrid();
	// update grid indices for all objects
	void UpdateGridInfo();

	/* Load */
	// 씬 파일에서 모든 객체와 조명의 정보를 불러온다.
	void LoadSceneObjects(const std::string& fileName);
	// 씬 파일에서 모든 객체의 정보를 불러온다. - call from Scene::LoadSceneObjects()
	void LoadGameObjects(std::ifstream& file);

	/* Other */
	// 태그별에 따라 객체를 초기화하고 씬 컨테이너에 객체를 삽입한다.(static, explosive, environments, ...)
	void InitObjectByTag(ObjectTag tag, sptr<GridObject> object);

#pragma endregion

#pragma region Render
public:
	// render scene
	void ClearRenderedObjects();
	void RenderShadow();
	void RenderCustomDepth();
	void ApplyDynamicContext();
	void RenderDynamicEnvironmentMappingObjects();
	void RenderDeferred();
	void RenderLights();
	void RenderFinal();
	void RenderForward();
	void RenderBloom();
	void RenderPostProcessing(int offScreenIndex, int outlineIndex);
	void RenderUI();
	void RenderDeferredForServer();
	void RenderText(RComPtr<struct ID2D1DeviceContext2> device);

private:
	// 카메라에 보이는 grid만 렌더링한다.
	// 투명, 빌보드 객체는 별도의 Shader를 사용해 렌더링해야 하므로 렌더링하지 않고 그 집합을 반환한다.
	// [renderedObjects]    : 렌더링된 모든 객체 (그리드에 포함된)
	// [transparentObjects] : 투명 객체
	// [billboardObjects]	: 빌보드 객체 (plane)
	void RenderGridObjects(RenderType type);
	void RenderSkinMeshObjects(RenderType type);
	void RenderInstanceObjects(RenderType type);
	void RenderObjectsWithFrustumCulling(std::set<GridObject*>& objects, RenderType type);
	void RenderEnvironments();
	void RenderTerrain(RenderType type);
	void RenderAfterSkinImage();

	// render [transparentObjects]
	void RenderDissolveObjects();
	void RenderSkyBox(RenderType type);
	void RenderParticles();
	void RenderAbilities();

	// [renderedObjects]와 grid의 bounds를 rendering한다.
	bool RenderBounds();
	void RenderObjectBounds();
	void RenderGridBounds();
#pragma endregion


#pragma region Update
public:
	void Start();
	void Update();

private:
	void ProcessCollisions();

	// update all objects
	void UpdateObjects();
	void AnimateObjects();
	void UpdateRenderedObjects();

#pragma endregion

public:
	// get objects[out] that collide with [collider] (expensive call cost)
	void CheckCollisionCollider(rsptr<Collider> collider, std::vector<GridObject*>& out, CollisionType type = CollisionType::All) const;
	float CheckCollisionsRay(int gridIndex, const Ray& ray) const;
	void ToggleDrawBoundings() { mIsRenderBounds = !mIsRenderBounds; }
	void ToggleFilterOptions();
	void SetFilterOptions(DWORD option);


	// update objects' grid indices
	void UpdateObjectGrid(GridObject* object, bool isCheckAdj = true);
	void RemoveObjectFromGrid(GridObject* object);

	void UpdateSurroundGrids();

	// create new game object from model
	sptr<GridObject> Instantiate(const std::string& modelName, ObjectTag tag = ObjectTag::Unspecified, ObjectLayer layer = ObjectLayer::Default, bool enable = true);

	void AddDynamicObject(rsptr<GridObject> object) { mDynamicObjects.push_back(object); }
	void RemoveDynamicObject(GridObject* object);

	sptr<ObjectPool> CreateObjectPool(const std::string& modelName, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr);
	sptr<ObjectPool> CreateObjectPool(rsptr<const MasterModel> model, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr);

	std::vector<sptr<Grid>> GetNeighborGrids(int gridIndex, bool includeSelf = false) const;

	void ToggleFullScreen();

	std::vector<sptr<GridObject>> FindObjectsByName(const std::string& name);

	// Unity Scene 스크립트 보유 객체에 대해 각각 Script를 Add 및 초기화 하도록 한다.
	void ProcessInitScriptOjbects(std::function<void(sptr<Object>)> processFunc);

	void UpdateTag(GridObject* object, ObjectTag beforeTag);

private:
	// do [processFunc] for activated objects
	void ProcessActiveObjects(std::function<void(sptr<GridObject>)> processFunc);
	// do [processFunc] for all objects
	void ProcessAllObjects(std::function<void(sptr<GridObject>)> processFunc);

	void RemoveDesrtoyedObjects();

	// move mObjectBuffer's objects to mDynamicObjects
	void PopObjectBuffer();

	bool IsGridOutOfRange(int index) { return index < 0 || index >= mGrids.size(); }

	// 유니티의 tag 문자열을 ObjectTag로 변환한다.
	static ObjectTag GetTagByString(const std::string& tag);

	// 유니티의 Layer 번호[num]를 ObjectLayer로 변환한다.
	static ObjectLayer GetLayerByNum(int num);
};
#pragma endregion