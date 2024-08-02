#include "EnginePch.h"
#include "BattleScene.h"
#include "DXGIMgr.h"
#include "FrameResource.h"

#include "Object.h"
#include "Model.h"
#include "Terrain.h"
#include "Shader.h"
#include "MeshRenderer.h"
#include "Timer.h"
#include "FileIO.h"
#include "Light.h"
#include "SkyBox.h"
#include "Texture.h"
#include "ObjectPool.h"
#include "ResourceMgr.h"
#include "AbilityMgr.h"
#include "ScriptExporter.h"

#include "Component/UI.h"
#include "Component/Camera.h"
#include "Component/Collider.h"
#include "Component/Component.h"
#include "Component/ParticleSystem.h"




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Getter
float BattleScene::GetTerrainHeight(float x, float z) const
{
	assert(mTerrain);

	return mTerrain->GetHeight(x, z);
}

std::vector<sptr<GameObject>> BattleScene::GetAllObjects() const
{
	std::vector<sptr<GameObject>> result;
	result.reserve(mStaticObjects.size() + mDynamicObjects.size());
	result.insert(result.end(), mStaticObjects.begin(), mStaticObjects.end());
	result.insert(result.end(), mDynamicObjects.begin(), mDynamicObjects.end());

	return result;
}

#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region DirectX
void BattleScene::UpdateAbilityCB(int& idx, const AbilityConstants& value)
{
	FRAME_RESOURCE_MGR->CopyData(idx, value);
}

void BattleScene::SetAbilityCB(int idx) const
{
	CMD_LIST->SetGraphicsRootConstantBufferView(DXGIMgr::I->GetGraphicsRootParamIndex(RootParam::Ability), FRAME_RESOURCE_MGR->GetAbilityCBGpuAddr(idx));
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Build
void BattleScene::Build()
{
	Scene::Build();
	std::cout << "Load Battle Scene...";

	// load models
	LoadSceneObjects();

	// build settings
	BuildTerrain();

	std::cout << "OK\n";

	Start();
}

void BattleScene::Release()
{
	Scene::Release();

	ProcessAllObjects([](sptr<Object> object) {
		object->Destroy();
		});

	mStaticObjects.clear();
	mDynamicObjects.clear();
	mObjectPools.clear();
	mDynamicObjectBuffer.clear();
	mScriptObjects.clear();
	mDestroyObjects.clear();
	mDissolveObjects.clear();
	mRenderedObjects.clear();
	mSkinMeshObjects.clear();
	mGridObjects.clear();
	mObjectsByShader.clear();

	mGrids.clear();
	mSurroundGrids.clear();

	mOpenList.clear();
	mClosedList.clear();

	mTerrain = nullptr;
}

void BattleScene::BuildTerrain()
{
	mTerrain = std::make_shared<Terrain>("Import/Terrain.bin");

	BuildGrid();
}

void BattleScene::BuildGrid()
{
	constexpr float kMaxHeight = 300.f;	// for 3D grid

	constexpr int gridCount = mkGridXCount * mkGridZCount;
	mGrids.resize(gridCount);

	// set grid bounds
	constexpr float gridExtentX = static_cast<float>(mkGridWidth) / 2.0f;
	constexpr float gridExtentZ = static_cast<float>(mkGridLength) / 2.0f;
	for (int z = 0; z < mkGridZCount; ++z) {
		for (int x = 0; x < mkGridXCount; ++x) {
			float gridX = (mkGridWidth * x) + ((float)mkGridWidth / 2);
			float gridZ = (mkGridLength * z) + ((float)mkGridLength / 2);

			BoundingBox bb{};
			bb.Center = Vec3(gridX, kMaxHeight, gridZ);
			bb.Extents = Vec3(gridExtentX, kMaxHeight, gridExtentZ);

			int index = (z * mkGridXCount) + x;
			mGrids[index] = std::make_shared<Grid>(index, mkGridWidth, mkGridLength, bb);
		}
	}
}

void BattleScene::UpdateGridInfo()
{
	ProcessActiveObjects([this](sptr<GridObject> object) {
		UpdateObjectGrid(object.get());
		});

	mTerrain->UpdateGrid();
}


void BattleScene::LoadSceneObjects()
{
	const std::string kFileName = "Import/Scene.bin";
	std::ifstream file = FileIO::OpenBinFile(kFileName);

	// load lights
	std::string token = FileIO::ReadString(file); // "<Lights>:"
	FileIO::ReadVal<int>(file);
	//

	LoadGameObjects(file);
}

void BattleScene::LoadGameObjects(std::ifstream& file)
{
	std::string token{};
	std::string name{};

	int objectCount;
	FileIO::ReadString(file, token); // "<GameObjects>:"
	FileIO::ReadVal(file, objectCount);

	mStaticObjects.reserve(objectCount);
	mDynamicObjects.reserve(objectCount);

	int sameObjectCount{};			// get one unique model from same object
	sptr<MasterModel> model{};
	sptr<ObjectPool> objectPool{};
	
	bool isInstancing{};
	ObjectTag tag{};

	for (int i = 0; i < objectCount; ++i) {
		sptr<GridObject> object{};

		if (sameObjectCount <= 0) {
			FileIO::ReadString(file, token); //"<Tag>:"
			FileIO::ReadString(file, token);
			tag = GetTagByString(token);
			
			int layerNum{};
			FileIO::ReadString(file, token); //"<Layer>:"
			FileIO::ReadVal(file, layerNum);

			FileIO::ReadString(file, token); //"<FileName>:"

			std::string meshName{};
			FileIO::ReadString(file, meshName);
			model = RESOURCE<MasterModel>(meshName);
			assert(model);

			FileIO::ReadString(file, token); //"<Transforms>:"
			FileIO::ReadVal(file, sameObjectCount);

			FileIO::ReadString(file, token); //"<IsInstancing>:"
			FileIO::ReadVal(file, isInstancing);
			
			if (isInstancing) {
				objectPool = CreateObjectPool(model, sameObjectCount, [&](rsptr<InstObject> object) {
					object->SetTag(tag);
					});
				if (tag == ObjectTag::Bound) {
					mBounds = objectPool;
				}
			}

			if (tag == ObjectTag::Untagged) {
				std::cout << "[WARNING] Untagged Object : " << model->GetName() << std::endl;
			}
		}

		if (sameObjectCount > 0) {
			if (isInstancing) {
				object = objectPool->Get(false);
			}
			else {
				object = std::make_shared<GridObject>();
				object->SetModel(model);
				InitObjectByTag(tag, object);
			}

			// transform
			{
				FileIO::ReadString(file, token);	// <Transform>:

				Matrix transform;
				FileIO::ReadVal(file, transform);
				object->SetWorldTransform(transform);

				FileIO::ReadString(file, token);	// </Transform>: or <ScriptExporter>:
				if (Hash(token) == Hash("<ScriptExporter>:"))
				{
					LoadScriptExporter(file, object);
					FileIO::ReadString(file); // </Transform>:
				}

				--sameObjectCount;
			}

			if (object->GetComponent<ScriptExporter>()) {
				mScriptObjects.push_back(object);
			}
		}
	}
}

void BattleScene::LoadScriptExporter(std::ifstream& file, rsptr<Object> object)
{
	auto scritpExorter = object->AddComponent<ScriptExporter>(false);
	scritpExorter->Load(file);
}

void BattleScene::InitObjectByTag(ObjectTag tag, sptr<GridObject> object)
{
	object->SetTag(tag);
	ObjectType type = object->GetType();

#ifdef SERVER_COMMUNICATION
	if (tag == ObjectTag::Enemy) {
		return;
	}
#endif

	switch (type) {
	case ObjectType::Dynamic:
	case ObjectType::DynamicMove:
		mDynamicObjects.push_back(object);
		break;
	default:
		mStaticObjects.push_back(object);
		break;
	}
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Render
void BattleScene::RenderBegin()
{
	ClearRenderedObjects();
}

void BattleScene::RenderShadow()
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	RenderGridObjects(RenderType::Shadow);
	RenderSkinMeshObjects(RenderType::Shadow);
	RenderInstanceObjects(RenderType::Shadow);
}


void BattleScene::RenderDeferred()
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	RenderGridObjects(RenderType::Deferred);
	RenderSkinMeshObjects(RenderType::Deferred);
	RenderInstanceObjects(RenderType::Deferred);

	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	RenderTerrain(RenderType::Deferred);
}


void BattleScene::RenderCustomDepth()
{
	if (!DXGIMgr::I->GetFilterOption(FilterOption::Custom))
		return;

	RenderSkinMeshObjects(RenderType::CustomDepth);
}

void BattleScene::RenderForward()
{
	RenderDissolveObjects();
	RenderAfterSkinImage();

	RenderAbilities();

	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	RenderParticles();
}

void BattleScene::RenderUI()
{
	Scene::RenderUI();

	RenderBounds();
}

void BattleScene::ApplyDynamicContext()
{
	DynamicEnvironmentMappingManager::I->Render(mSkinMeshObjects);
}

void BattleScene::RenderEnd()
{
	Scene::RenderEnd();

	ClearRenderedObjects();
}

void BattleScene::RenderDynamicEnvironmentMappingObjects()
{
	RenderGridObjects(RenderType::DynamicEnvironmentMapping);
	RenderInstanceObjects(RenderType::DynamicEnvironmentMapping);
	RenderTerrain(RenderType::DynamicEnvironmentMapping);
	RenderSkyBox(RenderType::DynamicEnvironmentMapping);
}


void BattleScene::RenderDeferredForServer()
{
	BattleScene::I->UpdateRenderedObjects();
}

void BattleScene::RenderTerrain(RenderType type)
{
	switch (type)
	{
	case RenderType::DynamicEnvironmentMapping:
		RESOURCE<Shader>("DEM_Terrain")->Set();
		break;
	default:
		RESOURCE<Shader>("Terrain")->Set();
		break;
	}

	if (mTerrain) {
		mTerrain->Render();
	}
}

void BattleScene::RenderAfterSkinImage()
{
	RESOURCE<Shader>("AfterSkinImage")->Set();

	for (auto& object : mObjectsByShader[ObjectTag::AfterSkinImage]) {
		object->Render();
	}
}

void BattleScene::RenderDissolveObjects()
{
	RESOURCE<Shader>("Dissolve")->Set();
	// [destroyTime]ï¿½ï¿½ ï¿½ï¿½ï¿? ï¿½ï¿½ ï¿½ï¿½Ã¼ ï¿½ï¿½ï¿½ï¿½
	constexpr float destroyTime = 1.f;
	std::set<sptr<GridObject>> destroyedObjects{};
	for (auto& it = mDissolveObjects.begin(); it != mDissolveObjects.end(); ++it) {
		auto& object = *it;

		object->mObjectCB.DeathElapsed += DeltaTime() / 2.f;
		object->Render();

		if (object->mObjectCB.DeathElapsed > destroyTime) {
			destroyedObjects.insert(object);
		}
	}

	for (auto& object : destroyedObjects) {
		mDissolveObjects.erase(object);
	}
}

void BattleScene::RenderSkyBox(RenderType type)
{
	switch (type)
	{
	case RenderType::DynamicEnvironmentMapping:
		RESOURCE<Shader>("DEM_SkyBox")->Set();
		break;
	default:
		RESOURCE<Shader>("SkyBox")->Set();
		break;
	}

	mSkyBox->Render();
}

void BattleScene::RenderParticles()
{
	ParticleManager::I->Render();
}

void BattleScene::RenderAbilities()
{
	AbilityMgr::I->Render();
}

void BattleScene::ClearRenderedObjects()
{
	mRenderedObjects.clear();
	mSkinMeshObjects.clear();
	mGridObjects.clear();
	mObjectsByShader.clear();
}

void BattleScene::RenderGridObjects(RenderType type)
{
	switch (type) {
	case RenderType::Shadow:
		RESOURCE<Shader>("Shadow_Global")->Set();
		break;
	case RenderType::Deferred:
		RESOURCE<Shader>("Global")->Set();
		break;
	case RenderType::DynamicEnvironmentMapping:
		RESOURCE<Shader>("DEM_Global")->Set();
		break;
	}

	RenderObjectsWithFrustumCulling(mGridObjects, type);
}

void BattleScene::RenderSkinMeshObjects(RenderType type)
{
	bool isRenderDissolve{};
	switch (type)
	{
	case RenderType::Shadow:
		RESOURCE<Shader>("Shadow_SkinMesh")->Set();
		isRenderDissolve = true;
		break;
	case RenderType::CustomDepth:
		RESOURCE<Shader>("CustomDepth_SkinMesh")->Set();
		isRenderDissolve = true;
		break;
	case RenderType::Deferred:
		RESOURCE<Shader>("SkinMesh")->Set();
		isRenderDissolve = false;
		break;
	case RenderType::DynamicEnvironmentMapping:
		RESOURCE<Shader>("DEM_SkinMesh")->Set();
		isRenderDissolve = false;
		break;
	}

	if (isRenderDissolve) {
		for (auto& object : mDissolveObjects) {
			object->Render();
		}
	}

	RenderObjectsWithFrustumCulling(mSkinMeshObjects, type);
}

void BattleScene::RenderInstanceObjects(RenderType type)
{
	switch (type)
	{
	case RenderType::Shadow:
		RESOURCE<Shader>("Shadow_ObjectInst")->Set();
		break;
	case RenderType::Deferred:
		RESOURCE<Shader>("ObjectInst")->Set();
		break;
	case RenderType::DynamicEnvironmentMapping:
		RESOURCE<Shader>("DEM_ObjectInst")->Set();
		break;
	}

	for (auto& buffer : mObjectPools) {
		buffer->Render();
	}
}

void BattleScene::RenderObjectsWithFrustumCulling(std::set<GridObject*>& objects, RenderType type)
{
	for (const auto& object : objects) {
		if (type == RenderType::Shadow && !object->GetUseShadow())
			continue;

		object->Render();
	}
}

bool BattleScene::RenderBounds()
{
	if (!mIsRenderBounds) {
		return false;
	}

	RESOURCE<Shader>("Wire")->Set();
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	RenderObjectBounds();
	RenderGridBounds();

	{
		mBounds->DoAllObjects([](rsptr<InstObject> object) {
			object->RenderBounds();
			});
	}

	return true;
}

void BattleScene::RenderObjectBounds()
{
	for (auto& object : mRenderedObjects) {
		object->RenderBounds();
	}
}

//#define DRAW_SCENE_GRID_3D
void BattleScene::RenderGridBounds()
{
	for (const auto& grid : mSurroundGrids) {
		constexpr float kGirdHeight = 0.5f;
		Vec3 pos = grid->GetBB().Center;
		pos.y = kGirdHeight;
		MeshRenderer::RenderPlane(pos, (float)mkGridWidth, (float)mkGridLength);
	}
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Update
void BattleScene::Start()
{
	std::cout << "Start(Build) Battle Scene...";

	/* Awake */
	mTerrain->Awake();
	ProcessAllObjects([](sptr<Object> object) {
		object->Awake();
		});

	std::cout << "Awake...";
	mManager->Awake();


	/* Enable */
	std::cout << "ActivateTerrain...";
	mTerrain->SetActive(true);

	std::cout << "ActivateObjects...";
	ProcessAllObjects([](sptr<Object> object) {
		object->SetActive(true);
		});
	mManager->SetActive(true);

	std::cout << "UpdatedGrid...";
	UpdateGridInfo();

	std::cout << "OK\n";

	Update();
}

void BattleScene::Update()
{
	// rendering //
	UpdateRenderedObjects();
	
	// collision //
	ProcessCollisions();

	// update objects //
	UpdateObjects();
	mManager->Update();
	mLight->Update();
	UpdateSurroundGrids();

	// update singletons //
	ParticleManager::I->Update();
	MainCamera::I->Update();
	Canvas::I->Update();
	MainCamera::I->LateUpdate();

	UpdateShaderVars();

	PopObjectBuffer();
}

void BattleScene::ProcessCollisions()
{
	for (const auto& grid : mSurroundGrids) {
		grid->CheckCollisions();
	}

	ProcessActiveObjects([this](sptr<Object> object) {
		if (object->IsActive()) {
			object->OnCollisionStay();
		}
		});
}

void BattleScene::CheckCollisionCollider(rsptr<Collider> collider, std::vector<GridObject*>& out, CollisionType type) const
{
	int gridIndex = GetGridIndexFromPos(collider->GetCenter());
	for (const auto& grid : GetNeighborGrids(gridIndex, true)) {
		if (!collider->Intersects(grid->GetBB())) {
			continue;
		}

		grid->CheckCollisions(collider, out, type);
	}
}

float BattleScene::CheckCollisionsRay(int gridIndex, const Ray& ray) const
{
	return mGrids[gridIndex]->CheckCollisionsRay(ray);
}

void BattleScene::UpdateObjects()
{
	ProcessActiveObjects([this](sptr<Object> object) {
		if (object->IsActive()) {
			object->Update();
		}
		});

	AnimateObjects();

	ProcessActiveObjects([this](sptr<Object> object) {
		if (object->IsActive()) {
			object->LateUpdate();
		}
		});
}

void BattleScene::AnimateObjects()
{
	for (auto& object : mRenderedObjects) {
		if (object->IsActive()) {
			object->Animate();
		}
	}
}

void BattleScene::UpdateRenderedObjects()
{
	if (mRenderedObjects.empty()) {
		for (const auto& grid : mGrids) {
			if (grid->Empty()) {
				continue;
			}

			if (MAIN_CAMERA->GetFrustumShadow().Intersects(grid->GetBB())) {
				auto& objects = grid->GetObjects();
				for (auto& object : objects) {
					if (object->GetTag() == ObjectTag::Bound) {
						continue;
					}

					const auto& collider = object->GetCollider();
					if (collider && MAIN_CAMERA->GetFrustumShadow().Intersects(collider->GetBS())) {
						mRenderedObjects.insert(object);
					}
				}
			}
		}

		std::set<GridObject*> disabledObjects{};
		for (auto& object : mRenderedObjects) {
			if (object->GetTag() == ObjectTag::AfterSkinImage) {
				mObjectsByShader[ObjectTag::AfterSkinImage].insert(object);
				continue;
			}
			else if (object->GetTag() == ObjectTag::Bound) {
				continue;
			}

			if (!object->IsActive()) {
				disabledObjects.insert(object);
				continue;
			}

			if (object->IsSkinMesh()) {
				mSkinMeshObjects.insert(object);
			}
			else {
				mGridObjects.insert(object);
			}
		}
		if (!disabledObjects.empty()) {
			std::set<GridObject*> diff;
			std::set_difference(mRenderedObjects.begin(), mRenderedObjects.end(), disabledObjects.begin(), disabledObjects.end(), std::inserter(diff, diff.begin()));
			mRenderedObjects = std::move(diff);
		}
	}
}
#pragma endregion





void BattleScene::PopObjectBuffer()
{
	if (!mDynamicObjectBuffer.empty()) {
		mDynamicObjects.insert(mDynamicObjects.end(), mDynamicObjectBuffer.begin(), mDynamicObjectBuffer.end());
		mDynamicObjectBuffer.clear();
	}
}

//////////////////* Others *//////////////////
rsptr<Grid> BattleScene::GetGridFromPos(const Vec3& pos)
{
	return mGrids[GetGridIndexFromPos(pos)];
}

int BattleScene::GetGridIndexFromPos(Vec3 pos) const
{
	const int gridX = static_cast<int>(pos.x / mkGridWidth);
	const int gridZ = static_cast<int>(pos.z / mkGridLength);

	return gridZ * mkGridXCount + gridX;
}

Pos BattleScene::GetTileUniqueIndexFromPos(const Vec3& pos) const
{
	const int tileGroupIndexX = static_cast<int>((pos.x) / Grid::mkTileWidth);
	const int tileGroupIndexZ = static_cast<int>((pos.z) / Grid::mkTileHeight);

	return Pos{ tileGroupIndexZ, tileGroupIndexX };
}

Vec3 BattleScene::GetTilePosFromUniqueIndex(const Pos& index) const
{
	const float posX = index.X * Grid::mkTileWidth;
	const float posZ = index.Z * Grid::mkTileHeight;

	return Vec3{ posX, 0, posZ };
}

Tile BattleScene::GetTileFromPos(const Vec3& pos) const
{
	return GetTileFromUniqueIndex(GetTileUniqueIndexFromPos(pos));
}

Tile BattleScene::GetTileFromUniqueIndex(const Pos& index) const
{
	const int gridX = static_cast<int>(index.X * Grid::mkTileWidth / mkGridWidth);
	const int gridZ = static_cast<int>(index.Z * Grid::mkTileHeight / mkGridLength);

	const int tileX = index.X % Grid::mTileCols;
	const int tileZ = index.Z % Grid::mTileRows;

	return mGrids[gridZ * mkGridXCount + gridX]->GetTileFromUniqueIndex(Pos{tileZ, tileX});
}

void BattleScene::SetTileFromUniqueIndex(const Pos& index, Tile tile)
{
	const int gridX = static_cast<int>(index.X * Grid::mkTileWidth / mkGridWidth);
	const int gridZ = static_cast<int>(index.Z * Grid::mkTileHeight / mkGridLength);

	const int tileX = index.X % Grid::mTileCols;
	const int tileZ = index.Z % Grid::mTileRows;

	mGrids[gridZ * mkGridXCount + gridX]->SetTileFromUniqueIndex(Pos{ tileZ, tileX }, tile);
}

void BattleScene::ToggleFilterOptions()
{
	static UINT8 filterIdx = 0;
	static std::array<DWORD, 6> values = { 0x004, 0x008, 0x010, 0x020, 0x080, 0x002 };
	DXGIMgr::I->SetFilterOptions(values[filterIdx++]);
	filterIdx %= values.size();

	if (filterIdx == 0)
		std::reverse(values.begin(), values.end());
}

void BattleScene::SetFilterOptions(DWORD option)
{
	DXGIMgr::I->SetFilterOptions(option);
}

void BattleScene::UpdateObjectGrid(GridObject* object, bool isCheckAdj)
{
	const int gridIndex = GetGridIndexFromPos(object->GetPosition());

	if (IsGridOutOfRange(gridIndex)) {
		RemoveObjectFromGrid(object);

		object->SetGridIndex(-1);
		return;
	}

	// remove object from current grid if move to another grid
	if (gridIndex != object->GetGridIndex()) {
		RemoveObjectFromGrid(object);
	}

	const auto& collider = object->GetCollider();
	if (collider && collider->IsActive()) {
		std::unordered_set<int> gridIndices{ gridIndex };
		const auto& objectBS = collider->GetBS();

		if (isCheckAdj && mGrids[gridIndex]->GetBB().Contains(objectBS) != ContainmentType::CONTAINS) {

			for (const auto& neighborGrid : GetNeighborGrids(gridIndex)) {
				if (neighborGrid->GetBB().Intersects(objectBS)) {
					neighborGrid->AddObject(object);
					gridIndices.insert(neighborGrid->GetIndex());
				}
				else {
					neighborGrid->RemoveObject(object);
				}
			}

			object->SetGridIndices(gridIndices);
		}
	}

	object->SetGridIndex(gridIndex);
	mGrids[gridIndex]->AddObject(object);
}

void BattleScene::RemoveObjectFromGrid(GridObject* object)
{
	for (const int gridIndex : object->GetGridIndices()) {
		if (!IsGridOutOfRange(gridIndex)) {
			mGrids[gridIndex]->RemoveObject(object);
		}
	}

	object->ClearGridIndices();
}

void BattleScene::UpdateSurroundGrids()
{
	const Vec3 cameraPos = MAIN_CAMERA->GetPosition();
	const int currGridIndex = GetGridIndexFromPos(cameraPos);

	static int prevGridIndex;
	if (prevGridIndex != currGridIndex) {
		mSurroundGrids.clear();
		mSurroundGrids = GetNeighborGrids(currGridIndex, true);
		prevGridIndex = currGridIndex;
	}
}

GridObject* BattleScene::Instantiate(const std::string& modelName, ObjectTag tag, bool enable)
{
	const auto& model = RESOURCE<MasterModel>(modelName);
	if (!model) {
		return nullptr;
	}

	sptr<GridObject> instance = std::make_shared<GridObject>();
	instance->SetModel(model);
	instance->SetTag(tag);
	instance->SetName(modelName);
	if (enable) {
		instance->SetActive(true);
	}
	mDynamicObjectBuffer.push_back(instance);

	return instance.get();
}


void BattleScene::RemoveDynamicObject(GridObject* target)
{
	for (size_t i = 0; i < mDynamicObjects.size();++i) {
		auto& object = mDynamicObjects[i];
		if (object.get() == target) {
			object->RemoveWholeComponents();
			if (object->IsSkinMesh()) {
				mDissolveObjects.insert(object);
			}
			mDestroyObjects.insert(i);

			{
				mRenderedObjects.erase(object.get());
				for (int idx : object->GetGridIndices()) {
					mGrids[idx]->RemoveObject(object.get());
				}
			}

			object = nullptr;
			return;
		}
	}
}


sptr<ObjectPool> BattleScene::CreateObjectPool(const std::string& modelName, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc)
{
	return CreateObjectPool(RESOURCE<MasterModel>(modelName), maxSize, objectInitFunc);
}

sptr<ObjectPool> BattleScene::CreateObjectPool(rsptr<const MasterModel> model, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc)
{
	sptr<ObjectPool> pool = mObjectPools.emplace_back(std::make_shared<ObjectPool>(model, maxSize));
	pool->CreateObjects<InstObject>(objectInitFunc);

	return pool;
}

std::vector<sptr<Grid>> BattleScene::GetNeighborGrids(int gridIndex, bool includeSelf) const
{
	std::vector<sptr<Grid>> result;
	result.reserve(9);

	const int gridX = gridIndex % mkGridXCount;
	const int gridZ = gridIndex / mkGridXCount;

	for (int offsetZ = -1; offsetZ <= 1; ++offsetZ) {
		for (int offsetX = -1; offsetX <= 1; ++offsetX) {
			const int neighborX = gridX + offsetX;
			const int neighborZ = gridZ + offsetZ;

			if (neighborX >= 0 && neighborX < mkGridXCount && neighborZ >= 0 && neighborZ < mkGridZCount) {
				const int neighborIndex = (neighborZ * mkGridXCount) + neighborX;

				if (neighborIndex == gridIndex && !includeSelf) {
					continue;
				}

				result.push_back(mGrids[neighborIndex]);
			}
		}
	}
	assert(!result.empty());

	return result;
}

void BattleScene::ToggleFullScreen()
{
	DXGIMgr::I->ToggleFullScreen();
}

std::vector<sptr<GridObject>> BattleScene::FindObjectsByName(const std::string& name)
{
	std::vector<sptr<GridObject>> result{};
	auto& FindObjects = [&](sptr<GridObject> object) {
		if (object->GetName() == name) {
			result.push_back(object);
		}
		};

	ProcessAllObjects(FindObjects);

	return result;
}

void BattleScene::ProcessInitScriptOjbects(std::function<void(sptr<Object>)> processFunc)
{
	for (const auto& object : mScriptObjects) {
		processFunc(object);
	}

	mScriptObjects.clear();
}

void BattleScene::UpdateTag(GridObject* object, ObjectTag beforeTag)
{
	for (int gridIndex : object->GetGridIndices()) {
		mGrids[gridIndex]->UpdateTag(object, beforeTag);
	}
}

void BattleScene::ProcessActiveObjects(std::function<void(sptr<GridObject>)> processFunc)
{
	for (auto& object : mDynamicObjects) {
		if (object && object->IsActive()) {
			processFunc(object);
		}
	}

	RemoveDesrtoyedObjects();

	for (auto& objectPool : mObjectPools) {
		objectPool->DoActiveObjects(processFunc);
	}
}

void BattleScene::ProcessAllObjects(std::function<void(sptr<GridObject>)> processFunc)
{
	for (auto& object : mStaticObjects) {
		processFunc(object);
	}

	for (auto& object : mDynamicObjects) {
		if (object) {
			processFunc(object);
		}
	}

	RemoveDesrtoyedObjects();

	for (auto& object : mObjectPools) {
		object->DoAllObjects(processFunc);
	}
}

void BattleScene::RemoveDesrtoyedObjects()
{
	for (auto& index : mDestroyObjects | std::ranges::views::reverse) {
		mDynamicObjects[index] = mDynamicObjects.back();
		mDynamicObjects.pop_back();
	}

	mDestroyObjects.clear();
}




ObjectTag BattleScene::GetTagByString(const std::string& tag)
{
	switch (Hash(tag)) {
	case Hash("Building"):
		return ObjectTag::Building;

	case Hash("Dissolve_Building"):
		return ObjectTag::DissolveBuilding;

	case Hash("Enemy"):
		return ObjectTag::Enemy;

	case Hash("Prop"):
		return ObjectTag::Prop;

	case Hash("Dynamic"):
		return ObjectTag::Dynamic;

	case Hash("Crate"):
		return ObjectTag::Crate;

	case Hash("Bound"):
		return ObjectTag::Bound;

	default:
		//assert(0);
		break;
	}

	return ObjectTag::Untagged;
}