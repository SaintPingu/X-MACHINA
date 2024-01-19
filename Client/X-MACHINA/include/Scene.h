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
	sptr<Shader> mBulletShader{};

	/* Object */
	sptr<GameObject> mWater{};
	std::vector<sptr<GameObject>> mEnvironments{};
	std::vector<sptr<GridObject>> mStaticObjects{};
	std::list<sptr<GridObject>> mExplosiveObjects{};		// dynamic
	std::list<sptr<GameObject>> mSpriteEffectObjects{};
	std::vector<sptr<ObjectPool>> mObjectPools{};

	/* Player */
	std::vector<sptr<GridObject>> mPlayers{};
	sptr<GridObject> mPlayer{};					// main player
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
	rsptr<GridObject> GetPlayer() const { return mPlayers.front(); }
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

	// [data]�� 32BitConstants�� Set�Ѵ�.
	void SetGraphicsRoot32BitConstants(RootParam param, const Matrix& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, const Vec4x4& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset);
	void SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset);

	// gpuAddr�� �ִ� CBV�� Set�Ѵ�.
	void SetGraphicsRootConstantBufferView(RootParam param, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr);

	// gpuAddr�� �ִ� SRV�� Set�Ѵ�.
	void SetGraphicsRootShaderResourceView(RootParam param, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr);

	// buffer(DepthStencil, ...)�� SRV ���ҽ��� �����Ѵ�.
	void CreateShaderResourceView(RComPtr<ID3D12Resource> resource, DXGI_FORMAT srvFormat);
	// texture�� SRV ���ҽ��� �����Ѵ�.
	void CreateShaderResourceView(Texture* texture, UINT descriptorHeapIndex);

private:
	void CreateGraphicsRootSignature();
	void CreateCbvSrvDescriptorHeaps(int cbvCount, int srvCount);

	void CreateShaderVars();
	void UpdateShaderVars();
	void UpdatePassConstants();
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
	void InitObjectByTag(const void* tag, sptr<GridObject> object);

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
	void RenderGridObjects(std::set<GridObject*>& renderedObjects, std::set<GridObject*>& transparentObjects, std::set<GridObject*>& billboardObjects);

	void RenderEnvironments();
	void RenderBullets();
	void RenderInstanceObjects();
	void RenderFXObjects();

	// render [billboards]
	void RenderBillboards(const std::set<GridObject*>& billboards);

	void RenderTerrain();

	// render [transparentObjects]
	void RenderTransparentObjects(const std::set<GridObject*>& transparentObjects);
	void RenderSkyBox();

	// [renderedObjects]�� grid�� bounds�� rendering�Ѵ�.
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

	int GetGridIndexFromPos(Vec3 pos);
	bool IsGridOutOfRange(int index) { return index < 0 || index >= mGrids.size(); }

	void DeleteExplodedObjects();
};
#pragma endregion
