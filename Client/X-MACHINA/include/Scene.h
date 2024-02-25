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
class Shader;
class InstShader;
class Camera;
class GameObject;
class GridObject;
class Terrain;
class Light;
class SkyBox;
class ObjectPool;
class Texture;
class AnimationClip;
class AnimatorController;
class TestCube;
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
	/* Model */
	std::unordered_map<std::string, sptr<const MasterModel>> mModels{};	// model folder에서 로드한 모델 객체 모음

	/* Light */
	sptr<Light> mLight{};

	/* Textures */
	std::unordered_map<std::string, sptr<Texture>> mTextureMap{};		// texture folder에서 로드한 texture 모음

	/* SkyBox */
	sptr<SkyBox> mSkyBox{};					// sky box object

	/* Shader */
	sptr<Shader> mGlobalShader{};			// 기본 Shader
	sptr<Shader> mBoundingShader{};
	sptr<Shader> mWaterShader{};
	sptr<Shader> mBillboardShader{};
	sptr<Shader> mSpriteShader{};
	sptr<Shader> mInstShader{};		// for InstObjects
	sptr<Shader> mTransparentShader{};
	sptr<Shader> mBulletShader{};
	sptr<Shader> mSkinnedMeshShader{};
	sptr<Shader> mFinalShader{};
	sptr<Shader> mLightingShader{};
	sptr<Shader> mOffScreenShader{};

	/* Object */
	sptr<GameObject> mWater{};
	std::vector<sptr<GameObject>> mEnvironments{};
	std::vector<sptr<GridObject>> mStaticObjects{};
	std::list<sptr<GridObject>> mExplosiveObjects{};		// dynamic
	std::list<sptr<GameObject>> mSpriteEffectObjects{};
	std::vector<sptr<ObjectPool>> mObjectPools{};

	std::set<GridObject*> mRenderedObjects{};
	std::set<GridObject*> mTransparentObjects{};
	std::set<GridObject*> mBillboardObjects{};
	std::set<GridObject*> mSkinMeshObjects{};

	/* Player */
	std::vector<sptr<GridObject>> mPlayers{};
	sptr<GridObject> mPlayer{};					// main player
	int	mCurrPlayerIndex{};						// main player index from [mPlayers]

	/* TestCube */
	std::vector<sptr<TestCube>> mTestCubes{};

	/* Map */
	sptr<Terrain> mTerrain{};
	BoundingBox mMapBorder{};					// max scene range	(grid will be generated within this border)

	/* Grid */
	std::vector<Grid>	mGrids{};				// all scene grids
	float				mGridStartPoint{};		// leftmost coord of the entire grid
	int					mGridhWidth{};			// length of x for one grid
	int					mGridCols{};			// number of columns in the grid

	/* Others */
	bool mIsRenderBounds = false;

	/* Animation */
	std::unordered_map<std::string, std::unordered_map<std::string, sptr<const AnimationClip>>>	mAnimationClipMap{};
	std::unordered_map<std::string, sptr<AnimatorController>>	mAnimatorControllerMap{};

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

	// [modelName]에 해당하는 MasterModel을 반환한다.
	rsptr<const MasterModel> GetModel(const std::string& modelName) const;
	// return the first inserted player
	rsptr<GridObject> GetPlayer() const { return mPlayers.front(); }
	// [name]에 해당하는 Texture 모델을 반환한다.
	rsptr<Texture> GetTexture(const std::string& name) const;

	sptr<const AnimationClip> GetAnimationClip(const std::string& folderName, const std::string& fileName) const { return mAnimationClipMap.at(folderName).at(fileName); }
	sptr<AnimatorController> GetAnimatorController(const std::string& controllerFile) const;
#pragma endregion


public:
#pragma region DirectX
public:
	void ReleaseUploadBuffers();

private:
	void UpdateShaderVars();
	void UpdateMainPassCB();
	void UpdateMaterialBuffer();

#pragma endregion


#pragma region Build
public:
	void BuildObjects();
	void ReleaseObjects();

private:
	/* Shader */
	// build all scene's shaders
	void BuildShaders();

	/* Shader */
	void BuildDeferredShader();
	void BuildForwardShader();

	/* Object */
	void BuildPlayers();
	void BuildTerrain();
	void BuildTestCube();

	/* Grid */
	// generate grids
	void BuildGrid();
	// update grid indices for all objects
	void UpdateGridInfo();

	/* Load */
	// 씬 파일에서 모든 객체와 조명의 정보를 불러온다.
	void LoadSceneObjects(const std::string& fileName);
	// 씬 파일에서 모든 객체의 정보를 불러온다. - call from Scene::LoadSceneObjects()
	void LoadGameObjects(FILE* file);
	// 유니티 씬에 없는(별도로 생성해야 하는) 동적 객체 모델을 불러온다.
	void LoadModels();

	void LoadAnimationClips();
	void LoadAnimatorControllers();

	/* Other */
	// 태그별에 따라 객체를 초기화하고 씬 컨테이너에 객체를 삽입한다.(static, explosive, environments, ...)
	void InitObjectByTag(const void* tag, sptr<GridObject> object);

#pragma endregion


#pragma region Render
public:
	// root signature, descriptor heap 등의 기본 정보를 설정한다.
	void OnPrepareRender();
	// render scene
	void RenderShadow();
	void RenderDeferred();
	void RenderLights();
	void RenderFinal();
	void RenderForward();
	void RenderUI();
	void RenderPostProcessing(int offScreenIndex);

private:
	// 카메라에 보이는 grid만 렌더링한다.
	// 투명, 빌보드 객체는 별도의 Shader를 사용해 렌더링해야 하므로 렌더링하지 않고 그 집합을 반환한다.
	// [renderedObjects]    : 렌더링된 모든 객체 (그리드에 포함된)
	// [transparentObjects] : 투명 객체
	// [billboardObjects]	: 빌보드 객체 (plane)
	void RenderGridObjects();

	void RenderSkinMeshObjects();
	void RenderEnvironments();
	void RenderBullets();
	void RenderInstanceObjects();
	void RenderFXObjects();
	void RenderTestCubes();

	// render [billboards]
	void RenderBillboards();

	void RenderTerrain();

	// render [transparentObjects]
	void RenderTransparentObjects(const std::set<GridObject*>& transparentObjects);
	void RenderSkyBox();

	// [renderedObjects]와 grid의 bounds를 rendering한다.
	bool RenderBounds(const std::set<GridObject*>& renderedObjects);
	void RenderObjectBounds(const std::set<GridObject*>& renderedObjects);
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

	// update all objects
	void UpdateObjects();
	// update effect objects
	void UpdateFXObjects();

	void UpdateSprites();
	// for dynamic(movable) lights
	void UpdateLights();
	void UpdateCamera();
#pragma endregion

public:
	void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

	void ToggleDrawBoundings();

	// create explosion effect at [pos]
	void CreateFX(FXType type, const Vec3& pos);
	void BlowAllExplosiveObjects();

	void ChangeToNextPlayer();
	void ChangeToPrevPlayer();

	// update objects' grid indices
	void UpdateObjectGrid(GridObject* object, bool isCheckAdj = true);
	void RemoveObjectFromGrid(GridObject* object);

private:
	// do [processFunc] for all objects
	void ProcessObjects(std::function<void(sptr<GridObject>)> processFunc);

	void CreateSpriteEffect(Vec3 pos, float speed, float scale = 1);
	void CreateSmallExpFX(const Vec3& pos);
	void CreateBigExpFX(const Vec3& pos);

	int GetGridIndexFromPos(Vec3 pos) const;
	bool IsGridOutOfRange(int index) { return index < 0 || index >= mGrids.size(); }

	void DeleteExplodedObjects();
};
#pragma endregion
