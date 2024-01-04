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
class InstShader;
class StaticShader;

class Camera;
class GameObject;
class Terrain;
class Light;
class Texture;
class SkyBox;
class GraphicsRootSignature;
class DescriptorHeap;
class ObjectPool;
#pragma endregion




#pragma region Class
class Scene : public Singleton<Scene> {
	friend class Singleton;

public:
	enum class FXType {
		SmallExplosion = 0,
		BigExplosion
	};

private:
	/* DirectX */
	sptr<GraphicsRootSignature> mGraphicsRootSignature{};

	/* Model */
	std::unordered_map<std::string, sptr<const MasterModel>> mModels{};	// model folder���� �ε��� �� ��ü ����

	/* Light */
	sptr<Light> mLight{};

	/* Textures */
	std::unordered_map<std::string, sptr<Texture>> mTextureMap{};		// texture folder���� �ε��� texture ����

	/* SkyBox */
	sptr<SkyBox> mSkyBox{};					// sky box object

	/* Shader */
	sptr<Shader> mGlobalShader{};			// �⺻ Shader
	sptr<Shader> mBoundingShader{};
	sptr<Shader> mWaterShader{};
	sptr<Shader> mBillboardShader{};
	sptr<Shader> mSpriteShader{};
	sptr<Shader> mInstShader{};		// for InstObjects
	sptr<Shader> mTransparentShader{};
	sptr<StaticShader> mSmallExpFXShader{};	// bullet fragments, ...
	sptr<StaticShader> mBigExpFXShader{};	// building fragments, ...

	/* Object */
	sptr<GameObject> mWater{};
	std::vector<sptr<GameObject>> mEnvironments{};
	std::vector<sptr<GameObject>> mStaticObjects{};
	std::list<sptr<GameObject>> mExplosiveObjects{};		// dynamic
	std::list<sptr<GameObject>> mSpriteEffectObjects{};
	std::vector<sptr<ObjectPool>> mInstanceBuffers{};

	/* Player */
	std::vector<sptr<GameObject>> mPlayers{};
	sptr<GameObject> mPlayer{};					// main player
	int	mCurrPlayerIndex{};						// main player index from [mPlayers]

	/* Map */
	sptr<Terrain> mTerrain{};
	BoundingBox mMapBorder{};					// max scene range	(grid will be generated within this border)

	/* Grid */
	std::vector<Grid>	mGrids{};				// all scene grids
	float				mGridStartPoint{};		// leftmost coord of the entire grid
	int					mGridhWidth{};			// length of x for one grid
	int					mGridCols{};			// number of columns in the grid

	/* Descriptor */
	sptr<DescriptorHeap> mDescriptorHeap{};

	/* Others */
	bool mIsRenderBounds = false;

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

	// [modelName]�� �ش��ϴ� MasterModel�� ��ȯ�Ѵ�.
	rsptr<const MasterModel> GetModel(const std::string& modelName) const;
	// return the first inserted player
	rsptr<GameObject> GetPlayer() const { return mPlayers.front(); }
	// [name]�� �ش��ϴ� Texture ���� ��ȯ�Ѵ�.
	rsptr<Texture> GetTexture(const std::string& name) const;

	RComPtr<ID3D12RootSignature> GetRootSignature() const;

	// [param]�� �ش��ϴ� root parameter index�� ��ȯ�Ѵ�.
	UINT GetRootParamIndex(RootParam param) const;
#pragma endregion


public:
#pragma region DirectX
public:
	void ReleaseUploadBuffers();

	// [data]�� 32BitConstants�� set�Ѵ�.
	void SetGraphicsRoot32BitConstants(RootParam param, const Matrix& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, const Vec4x4& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset);

	// buffer(DepthStencil, ...)�� SRV ���ҽ��� �����Ѵ�.
	void CreateShaderResourceView(RComPtr<ID3D12Resource> resource, DXGI_FORMAT srvFormat);
	// texture�� SRV ���ҽ��� �����Ѵ�.
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
	// build all scene's shaders
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
	// generate grids
	void BuildGrid();
	// update grid indices for all objects
	void UpdateGridInfo();

	/* Load */
	// �� ���Ͽ��� ��� ��ü�� ������ ������ �ҷ��´�.
	void LoadSceneObjects(const std::string& fileName);
	// �� ���Ͽ��� ��� ��ü�� ������ �ҷ��´�. - call from Scene::LoadSceneObjects()
	void LoadGameObjects(FILE* file);
	// ����Ƽ ���� ����(������ �����ؾ� �ϴ�) ���� ��ü ���� �ҷ��´�.
	void LoadModels();

	/* Other */
	// �±׺��� ���� ��ü�� �ʱ�ȭ�ϰ� �� �����̳ʿ� ��ü�� �����Ѵ�.(static, explosive, environments, ...)
	void InitObjectByTag(const void* tag, sptr<GameObject> object);

#pragma endregion


#pragma region Render
public:
	// root signature, descriptor heap ���� �⺻ ������ �����Ѵ�.
	void OnPrepareRender();
	// render scene
	void Render();

private:
	// ī�޶� ���̴� grid�� �������Ѵ�.
	// ����, ������ ��ü�� ������ Shader�� ����� �������ؾ� �ϹǷ� ���������� �ʰ� �� ������ ��ȯ�Ѵ�.
	// [renderedObjects]    : �������� ��� ��ü (�׸��忡 ���Ե�)
	// [transparentObjects] : ���� ��ü
	// [billboardObjects]	: ������ ��ü (plane)
	void RenderGridObjects(std::set<GameObject*>& renderedObjects, std::set<GameObject*>& transparentObjects, std::set<GameObject*>& billboardObjects);

	void RenderEnvironments();
	void RenderBullets();
	void RenderInstanceObjects();
	void RenderFXObjects();

	// render [billboards]
	void RenderBillboards(const std::set<GameObject*>& billboards);

	void RenderTerrain();

	// render [transparentObjects]
	void RenderTransparentObjects(const std::set<GameObject*>& transparentObjects);
	void RenderSkyBox();

	// [renderedObjects]�� grid�� bounds�� rendering�Ѵ�.
	bool RenderBounds(const std::set<GameObject*>& renderedObjects);
	void RenderObjectBounds(const std::set<GameObject*>& renderedObjects);
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
	// update each single object
	void UpdateObject(GameObject* object);
	// update effect objects (StaticShader)
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
	void UpdateObjectGrid(GameObject* object, bool isCheckAdj = true);
	void RemoveObjectFromGrid(GameObject* object);

private:
	// do [processFunc] for all objects
	void ProcessObjects(std::function<void(sptr<GameObject>)> processFunc);

	void CreateSpriteEffect(Vec3 pos, float speed, float scale = 1);
	void CreateSmallExpFX(const Vec3& pos);
	void CreateBigExpFX(const Vec3& pos);

	int GetGridIndexFromPos(Vec3 pos);
	bool IsGridOutOfRange(int index) { return index < 0 || index >= mGrids.size(); }

	void DeleteExplodedObjects();
};
#pragma endregion
