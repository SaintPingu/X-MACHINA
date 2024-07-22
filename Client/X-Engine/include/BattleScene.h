#pragma once


#pragma region Include
#include "Grid.h"
#include "Scene.h"
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

#pragma region Class
class BattleScene : public Singleton<BattleScene>, public Scene {
	friend Singleton;

public:
	enum class FXType {
		SmallExplosion = 0,
		BigExplosion
	};

private:
	/* Object */
	std::vector<sptr<GameObject>>	mEnvironments{};
	std::vector<sptr<GridObject>>	mStaticObjects{};
	std::vector<sptr<GridObject>>	mDynamicObjects{};
	std::vector<sptr<ObjectPool>>	mObjectPools{};
	std::vector<sptr<GridObject>>	mDynamicObjectBuffer{};		// �߰�(Instantiate) ��� ����
	std::vector<sptr<Object>>		mScriptObjects{};			// Unity Scene���� ��ũ��Ʈ�� ������ �ִ� ��ü��
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
	BattleScene();
	virtual ~BattleScene() = default;

#pragma endregion

public:
#pragma region Getter
	float GetTerrainHeight(float x, float z) const;
	std::vector<sptr<GameObject>> GetAllObjects() const;
	std::set<GridObject*> GetRenderedObjects() const { return mRenderedObjects; }

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
	void UpdateAbilityCB(int& idx, const AbilityConstants& value);
	void SetAbilityCB(int idx) const;
#pragma endregion

#pragma region Build
public:
	virtual void Build() override;
	virtual void Release() override;

	// ScriptExporter ������ �ε��Ѵ�.
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
	// �� ���Ͽ��� ��� ��ü�� ������ ������ �ҷ��´�.
	void LoadSceneObjects(const std::string& fileName);
	// �� ���Ͽ��� ��� ��ü�� ������ �ҷ��´�. - call from Scene::LoadSceneObjects()
	void LoadGameObjects(std::ifstream& file);

	/* Other */
	// �±׺��� ���� ��ü�� �ʱ�ȭ�ϰ� �� �����̳ʿ� ��ü�� �����Ѵ�.(static, explosive, environments, ...)
	void InitObjectByTag(ObjectTag tag, sptr<GridObject> object);

#pragma endregion

#pragma region Render
public:
	// render scene
	virtual void RenderBegin() override;
	virtual void RenderShadow() override;
	virtual void RenderDeferred() override;
	virtual void RenderCustomDepth() override;
	virtual void RenderForward() override;
	virtual void RenderUI() override;
	virtual void ApplyDynamicContext() override;
	void RenderDynamicEnvironmentMappingObjects();

	void RenderDeferredForServer();

private:
	// ī�޶� ���̴� grid�� �������Ѵ�.
	// ����, ������ ��ü�� ������ Shader�� ����� �������ؾ� �ϹǷ� ���������� �ʰ� �� ������ ��ȯ�Ѵ�.
	// [renderedObjects]    : �������� ��� ��ü (�׸��忡 ���Ե�)
	// [transparentObjects] : ���� ��ü
	// [billboardObjects]	: ������ ��ü (plane)
	void ClearRenderedObjects();
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

	// [renderedObjects]�� grid�� bounds�� rendering�Ѵ�.
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
	GridObject* Instantiate(const std::string& modelName, ObjectTag tag = ObjectTag::Unspecified, ObjectLayer layer = ObjectLayer::Default, bool enable = true);

	void AddDynamicObject(rsptr<GridObject> object) { mDynamicObjects.push_back(object); }
	void RemoveDynamicObject(GridObject* object);

	sptr<ObjectPool> CreateObjectPool(const std::string& modelName, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr);
	sptr<ObjectPool> CreateObjectPool(rsptr<const MasterModel> model, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc = nullptr);

	std::vector<sptr<Grid>> GetNeighborGrids(int gridIndex, bool includeSelf = false) const;

	void ToggleFullScreen();

	std::vector<sptr<GridObject>> FindObjectsByName(const std::string& name);

	// Unity Scene ��ũ��Ʈ ���� ��ü�� ���� ���� Script�� Add �� �ʱ�ȭ �ϵ��� �Ѵ�.
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

	// ����Ƽ�� tag ���ڿ��� ObjectTag�� ��ȯ�Ѵ�.
	static ObjectTag GetTagByString(const std::string& tag);

	// ����Ƽ�� Layer ��ȣ[num]�� ObjectLayer�� ��ȯ�Ѵ�.
	static ObjectLayer GetLayerByNum(int num);
};
#pragma endregion