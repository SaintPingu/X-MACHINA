#pragma region Include
#include "stdafx.h"
#include "Scene.h"
#include "DXGIMgr.h"

#include "UI.h"
#include "Object.h"
#include "Model.h"
#include "Terrain.h"
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Timer.h"
#include "FileIO.h"
#include "Light.h"
#include "Collider.h"
#include "InputMgr.h"
#include "SkyBox.h"
#include "Texture.h"
#include "RootSignature.h"
#include "DescriptorHeap.h"

#include "Script_Player.h"
#include "Script_ExplosiveObject.h"
#include "Script_Billboard.h"
#include "Script_Sprite.h"
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region C/Dtor
namespace {
	uptr<Scene> sceneInstance{};
}

Scene::Scene()
{
	constexpr int gridLengthCount{ 20 };		// gridCount = n*n

	constexpr Vec3 borderPos     = Vec3(256, 200, 256);
	constexpr Vec3 borderExtents = Vec3(1550, 500, 1550);

	mMapBorder      = { borderPos, borderExtents };	// map segmentation criteria
	mGridLength     = static_cast<int>(mMapBorder.Extents.x / gridLengthCount);
	mLight          = std::make_unique<Light>();
	mDescriptorHeap = std::make_shared<DescriptorHeap>();
}

void Scene::Create()
{
	sceneInstance = std::make_unique<Scene>();
}

void Scene::Destroy()
{
	sceneInstance = nullptr;
}

void Scene::Release()
{
	scene->ReleaseShaderVars();

	mainCameraObject->Destroy();
	canvas->Release();
	Destroy();
}

Scene* Scene::Inst()
{
	return sceneInstance.get();
}
#pragma endregion




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Getter
float Scene::GetTerrainHeight(float x, float z) const
{
	return (mTerrain) ? mTerrain->GetHeight(x, z) : 0.f;
}

rsptr<const MasterModel> Scene::GetModel(const std::string& modelName) const
{
	assert(mModels.contains(modelName));

	return mModels.at(modelName);
}

rsptr<Material> Scene::GetMaterial(const std::string& name) const
{
	assert(mMaterialMap.contains(name));
	
	return mMaterialMap.at(name);
}

rsptr<Texture> Scene::GetTexture(const std::string& name) const
{
	assert(mMaterialMap.contains(name));
	
	return mMaterialMap.at(name)->mTexture;
}

RComPtr<ID3D12RootSignature> Scene::GetRootSignature() const
{
	return mGraphicsRootSignature->Get();
}

UINT Scene::GetRootParamIndex(RootParam param) const
{
	return mGraphicsRootSignature->GetRootParamIndex(param);
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region DirectX
void Scene::ReleaseUploadBuffers()
{
	ProcessObjects([](sptr<GameObject> object) {
		object->ReleaseUploadBuffers();
		});

	MeshRenderer::ReleaseUploadBuffers();
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, const Matrix& data, UINT offset)
{
	constexpr UINT num32Bit = 16U;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, const Vec4x4& data, UINT offset)
{
	constexpr UINT num32Bit = 16U;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset)
{
	constexpr UINT num32Bit = 4U;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset)
{
	constexpr UINT num32Bit = 1U;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}

void Scene::CreateShaderResourceView(RComPtr<ID3D12Resource> resource, DXGI_FORMAT dxgiSrvFormat)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format                        = dxgiSrvFormat;
	srvDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels           = 1;
	srvDesc.Texture2D.MostDetailedMip     = 0;
	srvDesc.Texture2D.PlaneSlice          = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

	mDescriptorHeap->CreateSrv(resource, &srvDesc);
}

void Scene::CreateShaderResourceView(Texture* texture, UINT descriptorHeapIndex)
{
	ComPtr<ID3D12Resource> resource         = texture->GetResource();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = texture->GetShaderResourceViewDesc();

	mDescriptorHeap->CreateSrv(resource, &srvDesc);
	texture->SetGpuDescriptorHandle(mDescriptorHeap->GetGPUSrvLastHandle());
}

void Scene::CreateGraphicsRootSignature()
{
	mGraphicsRootSignature = std::make_shared<GraphicsRootSignature>();

	// 자주 사용되는 것을 앞에 배치할 것. (빠른 메모리 접근)
	mGraphicsRootSignature->Push(RootParam::GameObjectInfo,		D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,	0, D3D12_SHADER_VISIBILITY_ALL,		33);
	mGraphicsRootSignature->Push(RootParam::Camera,				D3D12_ROOT_PARAMETER_TYPE_CBV,				1, D3D12_SHADER_VISIBILITY_ALL);
	mGraphicsRootSignature->Push(RootParam::Light,				D3D12_ROOT_PARAMETER_TYPE_CBV,				2, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->Push(RootParam::GameInfo,			D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,	3, D3D12_SHADER_VISIBILITY_ALL,		1);
	mGraphicsRootSignature->Push(RootParam::SpriteInfo,			D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,	4, D3D12_SHADER_VISIBILITY_VERTEX,	16);

	mGraphicsRootSignature->Push(RootParam::Instancing,			D3D12_ROOT_PARAMETER_TYPE_SRV,				0, D3D12_SHADER_VISIBILITY_VERTEX);

	mGraphicsRootSignature->PushTable(RootParam::Texture,		D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->PushTable(RootParam::Texture1,		D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->PushTable(RootParam::Texture2,		D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->PushTable(RootParam::Texture3,		D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->PushTable(RootParam::RenderTarget,	D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 5, D3D12_SHADER_VISIBILITY_PIXEL);

	mGraphicsRootSignature->AddAlias(RootParam::Texture,	RootParam::TerrainLayer0);
	mGraphicsRootSignature->AddAlias(RootParam::Texture1,	RootParam::TerrainLayer1);
	mGraphicsRootSignature->AddAlias(RootParam::Texture2,	RootParam::TerrainLayer2);
	mGraphicsRootSignature->AddAlias(RootParam::Texture3,	RootParam::SplatMap);

	mGraphicsRootSignature->Create();
}


void Scene::CreateShaderVars()
{
	mLight->CreateShaderVars();
}

void Scene::UpdateShaderVars()
{
	mLight->UpdateShaderVars();
	mainCamera->UpdateShaderVars();
}

void Scene::ReleaseShaderVars()
{
	mLight->ReleaseShaderVars();
}

void Scene::CreateCbvSrvDescriptorHeaps(int cbvCount, int srvCount)
{
	mDescriptorHeap->Create(cbvCount, srvCount);
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Build
void Scene::BuildObjects()
{
	CreateGraphicsRootSignature();

	assert(mGraphicsRootSignature);
	CreateCbvSrvDescriptorHeaps(0, 1024);

	// load textures
	LoadTextures();

	// load canvas (UI)
	canvas->Init();

	// load models
	LoadSceneObjectsFromFile("Models/Scene.bin");
	LoadModels();

	// build settings
	BuildPlayers();
	BuildTerrain();

	// shader variables
	BuildShaders();
	CreateShaderVars();

	MeshRenderer::BuildMeshes();

	// skybox
	mSkyBox = std::make_shared<SkyBox>();
}

void Scene::ReleaseObjects()
{
	mGraphicsRootSignature = nullptr;
	MeshRenderer::Release();
}


void Scene::BuildShaders()
{
	BuildGlobalShader();
	BuildBoundingShader();
	BuildSmallExpFXShader();
	BuildBigExpFXShader();
	BuildBillboardShader();
}

void Scene::BuildGlobalShader()
{
	mGlobalShader = std::make_shared<TexturedShader>();
	mGlobalShader->Create();

	mWaterShader = std::make_shared<WaterShader>();
	mWaterShader->Create();

	mInstancingShader = std::make_shared<ObjectInstancingShader>();
	mInstancingShader->Create();

	mTransparentShader = std::make_shared<TransparentShader>();
	mTransparentShader->Create();
}

void Scene::BuildBoundingShader()
{
	mBoundingShader = std::make_shared<WireShader>();
	mBoundingShader->Create();
}

void Scene::BuildSmallExpFXShader()
{
	mSmallExpFXShader = std::make_shared<SmallExpEffectShader>();
	mSmallExpFXShader->Create();
}

void Scene::BuildBigExpFXShader()
{
	mBigExpFXShader = std::make_shared<BigExpEffectShader>();
	mBigExpFXShader->Create();
}

void Scene::BuildBillboardShader()
{
	mBillboardShader = std::make_shared<BillboardShader>();
	mBillboardShader->Create();

	mSpriteShader = std::make_shared<SpriteShader>();
	mSpriteShader->Create();
}

void Scene::BuildPlayers()
{
	mPlayers.reserve(1);
	sptr<GameObject> airplanePlayer = std::make_shared<GameObject>();
	airplanePlayer->AddComponent<Script_AirplanePlayer>()->CreateBullets(GetModel("tank_bullet"));
	airplanePlayer->SetModel(GetModel("Gunship"));

	mPlayers.emplace_back(airplanePlayer);

	mPlayer = mPlayers.front();
}

void Scene::BuildTerrain()
{
	constexpr int terrainGridLength = (TERRAIN_LENGTH - 1) / 8 + 1; // (512 / 8) = 64, 64 + 1 = 65

	mTerrain = std::make_shared<HeightMapTerrain>(_T("HeightMap.raw"), TERRAIN_LENGTH, TERRAIN_LENGTH, terrainGridLength, terrainGridLength);

	BuildGrid();
}

void Scene::BuildGrid()
{
	constexpr float maxHeight = 300.f;	// for 3D grid

	// recalculate scene grid size
	const int adjusted = Math::GetNearestMultiple(mMapBorder.Extents.x, mGridLength);
	mMapBorder.Extents = Vec3(adjusted, mMapBorder.Extents.y, adjusted);

	// set grid start pos
	mGridStartPoint = mMapBorder.Center.x - mMapBorder.Extents.x / 2;

	// set grid count
	int gridLengthCount = adjusted / mGridLength;
	int gridCount       = gridLengthCount;
	gridCount          *= gridCount;
	mGrids.resize(gridCount);
	mGridCols = static_cast<int>(sqrt(mGrids.size()));

	// set grid bounds
	float gridExtent = static_cast<float>(mGridLength) / 2.0f;
	for (int y = 0; y < gridLengthCount; ++y) {
		for (int x = 0; x < gridLengthCount; ++x) {
			int gridX = (mGridLength * x) + (mGridLength / 2) + mGridStartPoint;
			int gridZ = (mGridLength * y) + (mGridLength / 2) + mGridStartPoint;

			BoundingBox bb{};
			bb.Center  = Vec3(gridX, maxHeight / 2, gridZ);
			bb.Extents = Vec3(gridExtent, maxHeight, gridExtent);

			int index = (y * gridLengthCount) + x;
			mGrids[index].Init(index, bb);
		}
	}
}


void Scene::BuildGridObjects()
{
	ProcessObjects([this](sptr<GameObject> object) {
		UpdateObjectGrid(object.get());
		});

	mTerrain->UpdateGrid();
}


void Scene::LoadTextures()
{
	std::vector<std::string> textureNames;

	FileIO::GetTextureNames(textureNames, "Models/Textures");

	MaterialLoadInfo info{};
	info.Albedo = Vec4(0.1f, .1f, .1f, 1.f);
	for (auto& textureName : textureNames) {
		// load texture
		sptr<Texture> texture = std::make_shared<Texture>(D3DResource::Texture2D);
		texture->LoadTexture(textureName);

		// apply to material
		sptr<Material> material = std::make_shared<Material>();
		material->SetTexture(texture);

		sptr<MaterialColors> materialColors = std::make_shared<MaterialColors>(info);
		material->SetMaterialColors(materialColors);

		mMaterialMap.insert(std::make_pair(textureName, material));
	}
}

void Scene::LoadSceneObjectsFromFile(const std::string& fileName)
{
	FILE* file = NULL;
	::fopen_s(&file, fileName.c_str(), "rb");
	assert(file);
	::rewind(file);

	mLight->BuildLights(file);
	LoadGameObjects(file);
}

void Scene::LoadGameObjects(FILE* file)
{
	std::string token{};
	std::string name{};

	int objectCount;
	FileIO::ReadString(file, token); // "<GameObjects>:"
	FileIO::ReadVal(file, objectCount);

	mStaticObjects.reserve(objectCount);

	int sameObjectCount{};			// get one unique model from same object
	sptr<MasterModel> model{};
	sptr<ObjectInstanceBuffer> instBuffer{};

	bool isInstancing{};
	ObjectTag tag{};
	ObjectLayer layer{};

	for (int i = 0; i < objectCount; ++i) {
		sptr<GameObject> object{};

		if (sameObjectCount <= 0) {
			FileIO::ReadString(file, token); //"<Tag>:"
			FileIO::ReadString(file, token);
			tag = GetTagByString(token);

			int layerNum{};
			FileIO::ReadString(file, token); //"<Layer>:"
			FileIO::ReadVal(file, layerNum);
			layer = GetLayerByNum(layerNum);

			FileIO::ReadString(file, token); //"<FileName>:"

			std::string meshName{};
			FileIO::ReadString(file, meshName);

			model = FileIO::LoadGeometryFromFile("Models/Meshes/" + meshName + ".bin");
			mModels.insert(std::make_pair(meshName, model));

			FileIO::ReadString(file, token); //"<Transforms>:"
			FileIO::ReadVal(file, sameObjectCount);

			FileIO::ReadString(file, token); //"<IsInstancing>:"
			FileIO::ReadVal(file, isInstancing);

			if (isInstancing) {
				instBuffer = std::make_shared<ObjectInstanceBuffer>();
				instBuffer->CreateShaderVars(sameObjectCount);
				instBuffer->SetModel(model);
				if (GetObjectType(tag) == ObjectType::Dynamic) {
					instBuffer->SetDynamic();
				}
				mInstanceBuffers.emplace_back(instBuffer);
			}
		}

		if (isInstancing) {
			object = std::make_shared<InstancinObject>();
		}
		else {
			object = std::make_shared<GameObject>();
		}

		InitObjectByTag(&tag, object);

		object->SetLayer(layer);
		if (layer == ObjectLayer::Water) {
			mBackgrounds.pop_back();
			mWater = object;
		}


		object->SetModel(model);

		Vec4x4 transform;
		FileIO::ReadVal(file, transform);
		object->SetWorldTransform(transform);

		if (isInstancing) {
			((InstancinObject*)object.get())->SetBuffer(instBuffer);
		}

		--sameObjectCount;
	}
}

void Scene::LoadModels()
{
	const std::vector<std::string> binModelNames = { "tank_bullet", "sprite_explosion", };

	sptr<MasterModel> model;
	for (auto& name : binModelNames) {
		if (!mModels.contains(name)) {
			model = FileIO::LoadGeometryFromFile("Models/Meshes/" + name + ".bin");
			if (name.substr(0, 6) == "sprite") {
				model->SetSprite();
			}
			mModels.insert(std::make_pair(name, model));
		}
	}
}


void Scene::InitObjectByTag(const void* pTag, sptr<GameObject> object)
{
	ObjectTag tag = *(ObjectTag*)pTag;
	object->SetTag(tag);

	switch (tag) {
	case ObjectTag::Unspecified:
		break;
	case ObjectTag::ExplosiveSmall:
	{
		mExplosiveObjects.emplace_back(object);
		object->SetFlyable(true);

		const auto& script = object->AddComponent<Script_ExplosiveObject>();
		script->SetFX([&](const Vec3& pos) { CreateSmallExpFX(pos); });
		return;
	}

	break;
	case ObjectTag::Tank:
	case ObjectTag::Helicopter:
	case ObjectTag::ExplosiveBig:
	{
		mExplosiveObjects.emplace_back(object);

		const auto& script = object->AddComponent<Script_ExplosiveObject>();
		script->SetFX([&](const Vec3& pos) { CreateBigExpFX(pos); });
		return;
	}

	break;
	case ObjectTag::Background:
	{
		mBackgrounds.emplace_back(object);
		object->SetFlyable(true);
		return;
	}

	break;
	case ObjectTag::Billboard:
	{
		object->AddComponent<Script_Billboard>();
		object->SetFlyable(true);
	}

	break;
	case ObjectTag::Sprite:
	{
		object->AddComponent<Script_Sprite>();
		object->SetFlyable(true);
		return;
	}

	break;
	default:
		break;
	}

	mStaticObjects.emplace_back(object);
	object->SetFlyable(true);
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Render
namespace {
	bool IsBehind(const Vec3& point, const Vec4& plane)
	{
		return XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&plane), _VECTOR(point))) < 0.f;
	}
}
void Scene::OnPrepareRender()
{
	cmdList->SetGraphicsRootSignature(GetRootSignature().Get());

	mainCamera->SetViewportsAndScissorRects();

	mDescriptorHeap->Set();

	UpdateShaderVars();

	// Game Info
	static float timeElapsed{};
	timeElapsed += DeltaTime();
	scene->SetGraphicsRoot32BitConstants(RootParam::GameInfo, timeElapsed, 0);
}

static constexpr D3D12_PRIMITIVE_TOPOLOGY kObjectPrimitiveTopology  = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
static constexpr D3D12_PRIMITIVE_TOPOLOGY kUIPrimitiveTopology      = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
static constexpr D3D12_PRIMITIVE_TOPOLOGY kTerrainPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
static constexpr D3D12_PRIMITIVE_TOPOLOGY kBoundsPrimitiveTopology  = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
void Scene::Render()
{
	std::set<GameObject*> opaqueObjects{};
	std::set<GameObject*> transparentObjects{};
	std::set<GameObject*> billboardObjects{};

	OnPrepareRender();
	cmdList->IASetPrimitiveTopology(kObjectPrimitiveTopology);
	mGlobalShader->Render();

	RenderGridObjects(opaqueObjects, transparentObjects, billboardObjects);
	RenderBackgrounds();
	RenderBullets();
	RenderInstanceObjects();
	RenderFXObjects();
	RenderBillboards(billboardObjects);

	cmdList->IASetPrimitiveTopology(kTerrainPrimitiveTopology);
	RenderTerrain();
	cmdList->IASetPrimitiveTopology(kObjectPrimitiveTopology);

	RenderTransparentObjects(transparentObjects);

	RenderSkyBox();

	if (RenderBounds(opaqueObjects)) {
		cmdList->IASetPrimitiveTopology(kUIPrimitiveTopology);
	}

	canvas->Render();
}

void Scene::RenderTerrain()
{
	if (mTerrain) {
		mTerrain->Render();
	}
}

void Scene::RenderTransparentObjects(const std::set<GameObject*>& transparentObjects)
{
	mTransparentShader->Render();
	for (auto& object : transparentObjects) {
		object->Render();
	}
	if (mWater) {
		mWaterShader->Render();
		mWater->Render();
	}
}


void Scene::RenderSkyBox()
{
	mSkyBox->Render();
}


void Scene::RenderGridObjects(std::set<GameObject*>& opaqueObjects, std::set<GameObject*>& transparentObjects, std::set<GameObject*>& billboardObjects)
{
	for (const auto& grid : mGrids) {
		if (grid.Empty()) {
			continue;
		}

		if (mainCamera->IsInFrustum(grid.GetBB())) {
			auto& objects = grid.GetObjects();
			opaqueObjects.insert(objects.begin(), objects.end());
		}
	}

	for (auto& object : opaqueObjects) {
		if (object->IsTransparent()) {
			transparentObjects.insert(object);
			continue;
		}

		switch (object->GetTag())
		{
		case ObjectTag::Bullet:
			break;
		case ObjectTag::Billboard:
		case ObjectTag::Sprite:
			billboardObjects.insert(object);
			break;
		default:
			object->Render();
			break;
		}
	}
}

void Scene::RenderInstanceObjects()
{
	mInstancingShader->Render();
	for (auto& buffer : mInstanceBuffers) {
		buffer->Render();
	}
}

void Scene::RenderFXObjects()
{
	if (mSmallExpFXShader) {
		mSmallExpFXShader->Render();
	}
	if (mBigExpFXShader) {
		mBigExpFXShader->Render();
	}
}

void Scene::RenderBackgrounds()
{
	for (auto& background : mBackgrounds) {
		background->Render();
	}
}


void Scene::RenderBullets()
{
	for (auto& player : mPlayers) {
		if (player->IsActive()) {
			player->GetComponent<Script_AirplanePlayer>()->RenderBullets();
		}
	}
}


bool Scene::RenderBounds(const std::set<GameObject*>& opaqueObjects)
{
	if (!mIsRenderBounds || !mBoundingShader) {
		return false;
	}

	cmdList->IASetPrimitiveTopology(kBoundsPrimitiveTopology);

	mBoundingShader->Render();
	RenderObjectBounds(opaqueObjects);
	RenderGridBounds();

	return true;
}


void Scene::RenderObjectBounds(const std::set<GameObject*>& opaqueObjects)
{
	for (auto& player : mPlayers) {
		if (player->IsActive()) {
			player->RenderBounds();
		}
	}

	for (auto& object : opaqueObjects) {
		object->RenderBounds();
	}
}

//#define DRAW_SCENE_GRID_3D
void Scene::RenderGridBounds()
{
	for (const auto& grid : mGrids) {
#ifdef DRAW_SCENE_GRID_3D
		MeshRenderer::Render(grid.GetBB());
#else
		constexpr float kGirdHeight = 30.f;
		Vec3 pos = grid.GetBB().Center;
		pos.y = kGirdHeight;
		MeshRenderer::RenderPlane(pos, mGridLength, mGridLength);
#endif
	}
}

void Scene::RenderBillboards(const std::set<GameObject*>& billboards)
{
	mBillboardShader->Render();
	for (auto& object : billboards) {
		object->Render();
	}
	mSpriteShader->Render();
	for (auto& object : mSpriteEffectObjects) {
		object->Render();
	}
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Update
void Scene::Start()
{
	mainCameraObject->Start();

	ProcessObjects([](sptr<GameObject> object) {
		object->Start();
		});
	mTerrain->Start();

	if (mSmallExpFXShader) {
		mSmallExpFXShader->Start();
	}

	if (mBigExpFXShader) {
		mBigExpFXShader->Start();
	}

	BuildGridObjects();
}

void Scene::Update()
{
	CheckCollisions();

	UpdateObjects();
	UpdateLights();
	UpdateCamera();
	canvas->Update();
}

void Scene::Animate()
{

}


void Scene::CheckCollisions()
{
	UpdatePlayerGrid();

	for (Grid& grid : mGrids) {
		grid.CheckCollisions();
	}

	DeleteExplodedObjects();
}

void Scene::UpdatePlayerGrid()
{
	for (auto& player : mPlayers) {
		UpdateObjectGrid(player.get());

		const std::list<sptr<GameObject>>* bullets = player->GetComponent<Script_AirplanePlayer>()->GetBullets();

		if (!bullets) {
			continue;
		}

		for (auto& bullet : *bullets) {
			UpdateObjectGrid(bullet.get(), true);
		}
	}
}


void Scene::UpdateObjects()
{
	ProcessObjects([this](sptr<GameObject> object) {
		UpdateObject(object.get());
		});

	UpdateFXObjects();
	UpdateSprites();
}

void Scene::UpdateObject(GameObject* object)
{
	if (!object) {
		return;
	}

	object->Update();

	if (object->GetType() == ObjectType::DynamicMove) {
		UpdateObjectGrid(object);
	}
}

void Scene::UpdateFXObjects()
{
	if (mSmallExpFXShader) {
		mSmallExpFXShader->Update();
	}

	if (mBigExpFXShader) {
		mBigExpFXShader->Update();
	}
}


void Scene::UpdateSprites()
{
	for (auto it = mSpriteEffectObjects.begin(); it != mSpriteEffectObjects.end(); ) {
		auto& object = *it;
		if (object->GetComponent<Script_Sprite>()->IsEnd()) {
			it = mSpriteEffectObjects.erase(it);
		}
		else {
			object->Update();
			++it;
		}
	}
}

void Scene::UpdateLights()
{
	// update dynamic lights here.
}

void Scene::UpdateCamera()
{
	mainCameraObject->Update();
}
#pragma endregion





//////////////////* Others *//////////////////
void Scene::CreateSpriteEffect(Vec3 pos, float speed, float scale)
{
	sptr<GameObject> effect = std::make_shared<GameObject>();
	effect->SetModel(GetModel("sprite_explosion"));
	effect->RemoveComponent<ObjectCollider>();
	const auto& script = effect->AddComponent<Script_Sprite>();

	script->SetSpeed(speed);
	script->SetScale(scale);

	pos.y += 2;	// 보정
	effect->SetPosition(pos);

	effect->Start();
	mSpriteEffectObjects.emplace_back(effect);
}

void Scene::CreateSmallExpFX(Vec3 pos)
{
	if (mSmallExpFXShader) {
		mSmallExpFXShader->SetActive(pos);
	}

	CreateSpriteEffect(pos, 0.0001f);
}


void Scene::CreateBigExpFX(Vec3 pos)
{
	if (mBigExpFXShader) {
		mBigExpFXShader->SetActive(pos);
	}

	CreateSpriteEffect(pos, 0.025f, 5.f);
}


int Scene::GetGridIndexFromPos(Vec3 pos)
{
	pos.x -= mGridStartPoint;
	pos.z -= mGridStartPoint;

	const int gridX = static_cast<int>(pos.x / mGridLength);
	const int gridZ = static_cast<int>(pos.z / mGridLength);

	return gridZ * mGridCols + gridX;
}


void Scene::SetObjectGridIndex(rsptr<GameObject> object, int gridIndex)
{
	object->SetGridIndex(gridIndex);

	constexpr int gridRange = 1;
	std::unordered_set<int> gridIndices{};
	gridIndices.insert(gridIndex);

	object->SetGridIndices(gridIndices);
}


void Scene::DeleteExplodedObjects()
{
	for (auto it = mExplosiveObjects.begin(); it != mExplosiveObjects.end(); ) {
		auto& object = *it;
		if (object->GetComponent<Script_ExplosiveObject>()->IsExploded()) {
			// remove objects in grid
			for (int index : object->GetGridIndices()) {
				mGrids[index].RemoveObject(object.get());
			}

			it = mExplosiveObjects.erase(it);
		}
		else {
			++it;
		}
	}
}


void Scene::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{

}


void Scene::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	switch (messageID)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_HOME:
			timer->Stop();
			break;
		case VK_END:
			timer->Start();
			break;
		case '0':
			scene->BlowAllExplosiveObjects();
			break;

		case VK_OEM_6:
			ChangeToNextPlayer();
			break;
		case VK_OEM_4:
			ChangeToPrevPlayer();
			break;

		case VK_F5:
			ToggleDrawBoundings();
			break;
		default:
			break;
		}
	}

	break;
	default:
		break;
	}
}


void Scene::ToggleDrawBoundings()
{
	mIsRenderBounds = !mIsRenderBounds;

	ProcessObjects([](sptr<GameObject> object) {
		object->ToggleDrawBoundings();
		});
}


void Scene::CreateExplosion(ExplosionType explosionType, const Vec3& pos)
{
	switch (explosionType) {
	case ExplosionType::Small:
		CreateSmallExpFX(pos);
		break;
	case ExplosionType::Big:
		CreateBigExpFX(pos);
		break;
	default:
		assert(0);
		break;
	}
}


void Scene::BlowAllExplosiveObjects()
{
	for (auto& object : mExplosiveObjects)
	{
		object->GetComponent<Script_ExplosiveObject>()->Explode();
		for (int index : object->GetGridIndices()) {
			mGrids[index].RemoveObject(object.get());
		}
	}
	mExplosiveObjects.clear();
}


void Scene::ChangeToNextPlayer()
{
	++mCurrPlayerIndex;
	if (mCurrPlayerIndex >= mPlayers.size()) {
		mCurrPlayerIndex = 0;
	}

	mPlayer = mPlayers[mCurrPlayerIndex];
}


void Scene::ChangeToPrevPlayer()
{
	--mCurrPlayerIndex;
	if (mCurrPlayerIndex < 0) {
		mCurrPlayerIndex = static_cast<int>(mPlayers.size() - 1);
	}

	mPlayer = mPlayers[mCurrPlayerIndex];
}

// 객체의 Grid정보를 업데이트한다.
void Scene::UpdateObjectGrid(GameObject* object, bool isCheckAdj)
{
	if (!object->IsActive()) {
		return;
	}

	const int gridIndex = GetGridIndexFromPos(object->GetPosition());

	if (IsGridOutOfRange(gridIndex)) {
		RemoveObjectFromGrid(object);

		object->SetGridIndex(-1);
		return;
	}

	// from grid to another grid
	if (gridIndex != object->GetGridIndex()) {
		RemoveObjectFromGrid(object);

		object->SetGridIndex(gridIndex);
	}

	// 1칸 이내의 인접한 그리드 충돌검사
	// BoundingSphere가 Grid 내부에 완전히 포함되면 검사 X
	const auto& collider = object->GetComponent<ObjectCollider>();
	if (!collider) {
		return;
	}

	std::unordered_set<int> gridIndices{};
	const auto& objectBS = collider->GetBS();
	if (isCheckAdj && mGrids[gridIndex].GetBB().Contains(objectBS) != ContainmentType::CONTAINS) {
		int gridX = gridIndex % mGridCols;
		int gridZ = gridIndex / mGridCols;

		for (int offsetZ = -1; offsetZ <= 1; ++offsetZ) {
			for (int offsetX = -1; offsetX <= 1; ++offsetX) {
				int neighborX = gridX + offsetX;
				int neighborZ = gridZ + offsetZ;

				// 인덱스가 전체 그리드 범위 내에 있는지 확인
				if (neighborX >= 0 && neighborX < mGridCols && neighborZ >= 0 && neighborZ < mGridCols) {
					int neighborIndex = neighborZ * mGridCols + neighborX;

					if (mGrids[neighborIndex].GetBB().Intersects(objectBS)) {
						mGrids[neighborIndex].AddObject(object);
						gridIndices.insert(neighborIndex);
					}
					else {
						mGrids[neighborIndex].RemoveObject(object);
					}
				}
			}
		}
	}
	else {
		gridIndices.insert(gridIndex);
		mGrids[gridIndex].AddObject(object);
	}

	object->SetGridIndices(gridIndices);
}


void Scene::RemoveObjectFromGrid(GameObject* object)
{
	for (int index : object->GetGridIndices()) {
		mGrids[index].RemoveObject(object);
	}

	object->ClearGridIndices();
}


void Scene::ProcessObjects(std::function<void(sptr<GameObject>)> processFunc)
{
	for (auto& player : mPlayers) {
		processFunc(player);
	}

	for (auto& object : mStaticObjects) {
		processFunc(object);
	}

	for (auto& object : mExplosiveObjects) {
		processFunc(object);
	}
}