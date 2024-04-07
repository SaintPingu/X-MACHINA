#pragma once


#pragma region Include
#include "Grid.h"
#pragma endregion


#pragma region Define
#define scene Scene::Inst()
#define gameManager scene->GetGameManager()
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
	std::vector<sptr<GridObject>>	mDynamicObjectBuffer{};		// �߰�(Instantiate) ��� ����

	std::set<GridObject*>	mRenderedObjects{};
	std::set<GridObject*>	mTransparentObjects{};
	std::set<GridObject*>	mBillboardObjects{};
	std::set<GridObject*>	mSkinMeshObjects{};

	/* TestCube */
	std::vector<sptr<GameObject>>	mParticles{};

	/* Map */
	sptr<Terrain>		mTerrain{};
	BoundingBox			mMapBorder{};			// max scene range	(grid will be generated within this border)
	NavMesh				mNavMesh{};

	/* Grid */
	std::vector<Grid>	mGrids{};				// all scene grids
	float				mGridStartPoint{};		// leftmost coord of the entire grid
	int					mGridWidth{};			// length of x for one grid
	int					mGridCols{};			// number of columns in the grid

	/* Others */
	bool mIsRenderBounds = false;

private:
#pragma region C/Dtor
	Scene();
	virtual ~Scene() = default;

public:
	void Release();
	void TestOutputTile();

private:
#pragma endregion

public:
#pragma region Getter
	float GetTerrainHeight(float x, float z) const;
	std::vector<sptr<GameObject>> GetAllObjects() const;
	rsptr<Object> GetGameManager() const { return mGameManager; }
	std::vector<sptr<GameObject>> GetAllPartilceSystems() const;

	int GetGridIndexFromPos(Vec3 pos) const;
	Pos GetTileUniqueIndexFromPos(const Vec3& pos) const;
	TileObjectType GetTileObjectTypeFromUniqueIndex(const Pos& index) const;

	Vec3 GetTilePosFromIndex(const Pos& index) const;
#pragma endregion

#pragma region DirectX
public:
	void ReleaseUploadBuffers();

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

private:
	/* Object */
	void BuildTerrain();
	void BuildNavMesh();
	void BuildTest();

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
	void RenderShadow();
	void RenderDeferred();
	void RenderLights();
	void RenderFinal();
	void RenderForward();
	void RenderPostProcessing(int offScreenIndex);
	void RenderUI();

private:
	// ī�޶� ���̴� grid�� �������Ѵ�.
	// ����, ������ ��ü�� ������ Shader�� ����� �������ؾ� �ϹǷ� ���������� �ʰ� �� ������ ��ȯ�Ѵ�.
	// [renderedObjects]    : �������� ��� ��ü (�׸��忡 ���Ե�)
	// [transparentObjects] : ���� ��ü
	// [billboardObjects]	: ������ ��ü (plane)
	void RenderGridObjects(bool isShadowed = false);
	void RenderSkinMeshObjects(bool isShadowed = false);
	void RenderEnvironments();
	void RenderInstanceObjects();

	void RenderTerrain();

	// render [transparentObjects]
	void RenderTransparentObjects(const std::set<GridObject*>& transparentObjects);
	void RenderSkyBox();
	void RenderParticles();

	// [renderedObjects]�� grid�� bounds�� rendering�Ѵ�.
	bool RenderBounds(const std::set<GridObject*>& renderedObjects);
	void RenderObjectBounds(const std::set<GridObject*>& renderedObjects);
	void RenderGridBounds();
#pragma endregion


#pragma region Update
public:
	void Start();
	void Update();

private:
	void CheckCollisions();

	// update all objects
	void UpdateObjects();

#pragma endregion

public:
	void ToggleDrawBoundings();

	// update objects' grid indices
	void UpdateObjectGrid(GridObject* object, bool isCheckAdj = true);
	void RemoveObjectFromGrid(GridObject* object);

	// create new game object from model
	sptr<GridObject> Instantiate(const std::string& modelName, bool enable = true);

	void AddDynamicObject(rsptr<GridObject> object) { mDynamicObjects.push_back(object); }

	sptr<ObjectPool> CreateObjectPool(const std::string& modelName, int maxSize, std::function<void(rsptr<InstObject>)> objectInitFunc = nullptr);
	sptr<ObjectPool> CreateObjectPool(rsptr<const MasterModel> model, int maxSize, std::function<void(rsptr<InstObject>)> objectInitFunc = nullptr);

private:
	// do [processFunc] for activated objects
	void ProcessActiveObjects(std::function<void(sptr<GridObject>)> processFunc);
	// do [processFunc] for all objects
	void ProcessAllObjects(std::function<void(sptr<GridObject>)> processFunc);

	// move mObjectBuffer's objects to mDynamicObjects
	void PopObjectBuffer();

	bool IsGridOutOfRange(int index) { return index < 0 || index >= mGrids.size(); }
};
#pragma endregion
