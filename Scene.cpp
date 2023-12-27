#include "stdafx.h"
#include "Scene.h"
#include "DXGIMgr.h"

#include "Object.h"
#include "Model.h"
#include "Terrain.h"
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Timer.h"
#include "FileMgr.h"
#include "Light.h"
#include "Collider.h"
#include "InputMgr.h"
#include "SkyBox.h"
#include "Texture.h"
#include "RootSignature.h"
#include "DescriptorHeap.h"

#include "UI.h"

#include "Script_Player.h"
#include "Script_ExplosiveObject.h"
#include "Script_Billboard.h"
#include "Script_Sprite.h"




//#define DRAW_SCENE_GRID_3D


// [ Scene ] //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////

static uptr<Scene> sceneInstance{};

void Scene::Create()
{
	sceneInstance = std::make_unique<Scene>();
}


void Scene::Destroy()
{
	sceneInstance = nullptr;
	Canvas::Inst()->OnDestroy();
}


Scene* Scene::Inst()
{
	return sceneInstance.get();
}



Scene::Scene()
{
	constexpr int gridLengthCount{ 20 };		// gridCount = n*n

	constexpr Vec3 borderPos = Vec3(256, 200, 256);
	constexpr Vec3 borderExtents = Vec3(1500, 500, 1500);

	mMapBorder = { borderPos, borderExtents };	// map segmentation criteria
	mGridLength = static_cast<int>(mMapBorder.Extents.x / gridLengthCount);
	mLight = std::make_unique<Light>();

	mDescriptorHeap = std::make_shared<DescriptorHeap>();
}


Scene::~Scene()
{

}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////
float Scene::GetTerrainHeight(float x, float z) const
{
	return (mTerrain) ? mTerrain->GetHeight(x, z) : 0.0f;
}


rsptr<const MasterModel> Scene::GetModel(const std::string& modelName)
{
	assert(mModels.contains(modelName));

	return mModels[modelName];
}


rsptr<Texture> Scene::GetTexture(const std::string& name)
{
	assert(mMaterialMap.contains(name)); return mMaterialMap[name]->mTexture;
}

rsptr<Camera> Scene::GetMainCamera() const
{
	return mMainCamera->GetCamera();
}

sptr<CameraObject> Scene::GetCameraObject() const
{
	return mMainCamera;
}

RComPtr<ID3D12RootSignature> Scene::GetRootSignature() const
{
	return mGraphicsRootSignature->Get();
}

UINT Scene::GetRootParamIndex(RootParam param)
{
	return mGraphicsRootSignature->GetRootParamIndex(param);
}

void Scene::SetGlobalShader() const
{
	mGlobalShader->Render();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////* DirectX *//////////////////
void Scene::CreateGraphicsRootSignature()
{
	mGraphicsRootSignature = std::make_shared<GraphicsRootSignature>();

	// 자주 사용되는 것을 앞에 배치할 것. (빠른 메모리 접근)
	mGraphicsRootSignature->Push(RootParam::GameObjectInfo,		D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,	0, D3D12_SHADER_VISIBILITY_ALL, 33);
	mGraphicsRootSignature->Push(RootParam::Camera,				D3D12_ROOT_PARAMETER_TYPE_CBV,				1, D3D12_SHADER_VISIBILITY_ALL);
	mGraphicsRootSignature->Push(RootParam::Light,				D3D12_ROOT_PARAMETER_TYPE_CBV,				2, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->Push(RootParam::GameInfo,			D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,	3, D3D12_SHADER_VISIBILITY_ALL, 1);
	mGraphicsRootSignature->Push(RootParam::SpriteInfo,			D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,	4, D3D12_SHADER_VISIBILITY_VERTEX, 16);

	mGraphicsRootSignature->Push(RootParam::Instancing,			D3D12_ROOT_PARAMETER_TYPE_SRV,				0, D3D12_SHADER_VISIBILITY_VERTEX);

	mGraphicsRootSignature->PushTable(RootParam::Texture,		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,			2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->PushTable(RootParam::Texture1,		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,			3, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->PushTable(RootParam::Texture2,		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,			4, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->PushTable(RootParam::Texture3,		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,			5, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->PushTable(RootParam::Texture4,		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,			6, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->PushTable(RootParam::RenderTarget,	D3D12_DESCRIPTOR_RANGE_TYPE_SRV,			7, 5, D3D12_SHADER_VISIBILITY_PIXEL);

	mGraphicsRootSignature->AddAlias(RootParam::Texture, RootParam::TerrainLayer0);
	mGraphicsRootSignature->AddAlias(RootParam::Texture1, RootParam::TerrainLayer1);
	mGraphicsRootSignature->AddAlias(RootParam::Texture2, RootParam::TerrainLayer2);
	mGraphicsRootSignature->AddAlias(RootParam::Texture3, RootParam::TerrainLayer3);
	mGraphicsRootSignature->AddAlias(RootParam::Texture4, RootParam::SplatMap);

	mGraphicsRootSignature->Create();
}


void Scene::CreateShaderVariables()
{
	mLight->CreateShaderVariables();
}


void Scene::UpdateShaderVariables()
{
	mLight->UpdateShaderVariables();
	mMainCamera->UpdateShaderVariables();
}


void Scene::ReleaseShaderVariables()
{
	mLight->ReleaseShaderVariables();
	mMainCamera->ReleaseShaderVariables();
}


void Scene::ReleaseUploadBuffers()
{
	ProcessObjects([](sptr<GameObject> object) {
		object->ReleaseUploadBuffers();
		});

	MeshRenderer::ReleaseStaticUploadBuffers();
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, const Matrix& data, UINT offset)
{
	constexpr UINT num32Bit = 16;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, const Vec4x4& data, UINT offset)
{
	constexpr UINT num32Bit = 16;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset)
{
	constexpr UINT num32Bit = 4;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset)
{
	constexpr UINT num32Bit = 1;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////* Build *//////////////////
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
	mGlobalShader->CreateShader();
	
	mWaterShader = std::make_shared<WaterShader>();
	mWaterShader->CreateShader();

	mInstancingShader = std::make_shared<ObjectInstancingShader>();
	mInstancingShader->CreateShader();

	mTransparentShader = std::make_shared<TransparentShader>();
	mTransparentShader->CreateShader();
}


void Scene::BuildBoundingShader()
{
	mBoundingShader = std::make_shared<WireShader>();
	mBoundingShader->CreateShader();
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
	mBillboardShader->CreateShader();
	
	mSpriteShader = std::make_shared<SpriteShader>();
	mSpriteShader->CreateShader();
}


void Scene::BuildPlayers()
{
	LIGHT_RANGE lightRange{};
	// [1]
	mPlayers.reserve(2);
	sptr<GameObject> airplanePlayer = std::make_shared<GameObject>();
	airplanePlayer->AddComponent<Script_AirplanePlayer>()->CreateBullets(GetModel("tank_bullet"));
	airplanePlayer->SetModel(GetModel("Gunship"));

	mPlayers.emplace_back(airplanePlayer);


	//// [2]
	//sptr<GameObject> tankPlayer = std::make_shared<GameObject>();
	//tankPlayer->AddComponent<Script_AirplanePlayer>()->CreateBullets(mGraphicsRootSignature, GetModel("tank_bullet"));
	//tankPlayer->GetComponent<Script_TankPlayer>()->SetSpawn(Vec3(50.0f, 0.0f, 50.0f));
	//tankPlayer->SetModel(GetModel("tank"));
	//
	//mPlayers.emplace_back(tankPlayer);


	//// [3]
	//tankPlayer = std::make_shared<CTankPlayer>(mGraphicsRootSignature);
	//tankPlayer->SetModel(GetModel("tank"));
	//tankPlayer->OnInitialize();
	//tankPlayer->CreateBullets(mGraphicsRootSignature, GetModel("tank_bullet"));
	//tankPlayer->CreateBoundingMeshes(mGraphicsRootSignature);

	//tankPlayer->SetSpawn(Vec3(453, 0.0f, 119));
	//tankPlayer->SetHP(300.0f);
	//tankPlayer->SetDamage(50.0f);
	//tankPlayer->SetFireDelay(1.0f);
	//tankPlayer->SetMovingSpeed(10.0f);
	//tankPlayer->SetRotationSpeed(35.0f);

	//mPlayers.emplace_back(tankPlayer);


	//// [4]
	//tankPlayer = std::make_shared<CTankPlayer>(mGraphicsRootSignature);
	//tankPlayer->SetModel(GetModel("tank"));
	//tankPlayer->OnInitialize();
	//tankPlayer->CreateBullets(mGraphicsRootSignature, GetModel("tank_bullet"));
	//tankPlayer->CreateBoundingMeshes(mGraphicsRootSignature);

	//tankPlayer->SetSpawn(Vec3(413, 0.0f, 441));
	//tankPlayer->SetHP(300.0f);
	//tankPlayer->SetDamage(5.0f);
	//tankPlayer->SetFireDelay(0.25f);
	//tankPlayer->SetMovingSpeed(25.0f);
	//tankPlayer->SetRotationSpeed(60.0f);

	//mPlayers.emplace_back(tankPlayer);


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
	constexpr float maxHeight = 300.0f;	// for 3D grid

	// recalculate scene grid size
	int adjusted = RoundToNearestMultiple(mMapBorder.Extents.x, mGridLength);
	mMapBorder.Extents = Vec3(adjusted, mMapBorder.Extents.y, adjusted);

	// set grid start pos
	mGridStartPoint = mMapBorder.Center.x - mMapBorder.Extents.x / 2;

	// set grid count
	int gridLengthCount = adjusted / mGridLength;
	int gridCount = gridLengthCount;
	gridCount *= gridCount;
	mGrids.resize(gridCount);
	mGridCols = static_cast<int>(sqrt(mGrids.size()));

	// set grid bounds
	float gridExtent = static_cast<float>(mGridLength) / 2.0f;
	for (int y = 0; y < gridLengthCount; ++y) {
		for (int x = 0; x < gridLengthCount; ++x) {
			int gridX = (mGridLength * x) + (mGridLength / 2) + mGridStartPoint;
			int gridZ = (mGridLength * y) + (mGridLength / 2) + mGridStartPoint;

			BoundingBox bb{};
			bb.Center = Vec3(gridX, maxHeight / 2, gridZ);
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

	for (auto& terrain : mTerrain->GetTerrains()) {
		UpdateObjectGrid(terrain.get());
	}
}


void Scene::BuildCamera()
{
	mMainCamera = std::make_shared<MainCamera>();
	mMainCamera->CreateShaderVariables();
}


void Scene::LoadModels()
{
	const std::vector<std::string> binModelNames = { "tank_bullet", "sprite_explosion", };

	sptr<MasterModel> model;
	for (auto& name : binModelNames) {
		if (!mModels.contains(name)) {
			model = FileMgr::LoadGeometryFromFile("Models/Meshes/" + name + ".bin");
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

		auto& script = object->AddComponent<Script_ExplosiveObject>();
		script->SetFX([&](const Vec3& pos) { CreateSmallExpFX(pos); });
		return;
	}

	break;
	case ObjectTag::Tank:
	case ObjectTag::Helicopter:
	case ObjectTag::ExplosiveBig:
	{
		mExplosiveObjects.emplace_back(object);

		auto& script = object->AddComponent<Script_ExplosiveObject>();
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

	mStatiObjects.emplace_back(object);
	object->SetFlyable(true);
}

void Scene::LoadGameObjects(FILE* file)
{
	std::string token{};
	std::string name{};

	UINT nReads = 0;

	int objectCount;
	::ReadUnityBinaryString(file, token); // "<GameObjects>:"
	nReads = (UINT)::fread(&objectCount, sizeof(int), 1, file);

	mStatiObjects.reserve(objectCount);

	int sameObjectCount{};			// get one unique model from same object
	sptr<MasterModel> model{};
	sptr<ObjectInstanceBuffer> instBuffer{};
	bool isInstancing{};
	ObjectTag tag{};
	ObjectLayer layer{};

	for (int i = 0; i < objectCount; ++i) {
		sptr<GameObject> object{};

		if (sameObjectCount <= 0) {
			::ReadUnityBinaryString(file, token); //"<Tag>:"
			::ReadUnityBinaryString(file, token);
			tag = GetTagByName(token);

			int layerNum{};
			::ReadUnityBinaryString(file, token); //"<Layer>:"
			::fread(&layerNum, sizeof(int), 1, file);
			layer = GetLayerByNum(layerNum);

			::ReadUnityBinaryString(file, token); //"<FileName>:"		// If an error occurs here, it may be a lighting problem.
			 
			std::string meshName{}; 
			::ReadUnityBinaryString(file, meshName);

			model = FileMgr::LoadGeometryFromFile("Models/Meshes/" + meshName + ".bin");
			mModels.insert(std::make_pair(meshName, model));
			 
			::ReadUnityBinaryString(file, token); //"<Transforms>:"
			::fread(&sameObjectCount, sizeof(int), 1, file);

			::ReadUnityBinaryString(file, token); //"<IsInstancing>:"
			::fread(&isInstancing, sizeof(bool), 1, file);
				
			if (isInstancing) {
				instBuffer = std::make_shared<ObjectInstanceBuffer>();
				instBuffer->CreateShaderVariables(sameObjectCount);
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
		if(layer == ObjectLayer::Water){
			mBackgrounds.pop_back();
			mWater = object;
		}
		

		object->SetModel(model);

		Vec4x4 transform;
		::fread(&transform, sizeof(float), 16, file); //Transform
		object->SetWorldTransform(transform);

		if (isInstancing) {
			((InstancinObject*)object.get())->SetBuffer(instBuffer);
		}

		--sameObjectCount;
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

void Scene::LoadTextures()
{
	std::vector<std::string> textureNames;
	
	::LoadTextureNames(textureNames, "Models/Textures");

	MATERIALLOADINFO info{}; 
	info.mAlbedo = Vec4(0.1f, .1f, .1f, 1.0f);
	for (auto& textureName : textureNames) {
		// load texture
		sptr<Texture> texture = std::make_shared<Texture>(RESOURCE_TEXTURE2D);
		texture->LoadTexture(textureName);

		// apply to material
		sptr<Material> material = std::make_shared<Material>();
		material->SetTexture(texture);

		sptr<MaterialColors> materialColors = std::make_shared<MaterialColors>(info);
		material->SetMaterialColors(materialColors);

		mMaterialMap.insert(std::make_pair(textureName, material));
	}
}


void Scene::CreateCbvSrvDescriptorHeaps(int cbvCount, int srvCount)
{
	mDescriptorHeap->Create(cbvCount, srvCount);
}


void Scene::BuildObjects()
{
	CreateGraphicsRootSignature();

	assert(mGraphicsRootSignature);
	CreateCbvSrvDescriptorHeaps(0, 1024);

	// load textures
	LoadTextures();

	// load canvas (UI)
	Canvas::Inst()->Create();

	// load models
	LoadSceneObjectsFromFile("Models/Scene.bin");
	LoadModels();

	// build settings
	BuildPlayers();
	BuildTerrain();

	// shader variables
	BuildShaders();
	CreateShaderVariables();

#ifdef DRAW_SCENE_GRID_3D
	mGridMesh = std::make_shared<ModelObjectMesh>((float)mGridLength, mMaxGridHeight, (float)mGridLength, false, true);
#else
	mGridMesh = std::make_shared<ModelObjectMesh>((float)mGridLength, (float)mGridLength, true);
#endif

	MeshRenderer::BuildMeshes();

	// camera
	BuildCamera();

	// skybox
	mSkyBox = std::make_shared<SkyBox>();
}


void Scene::ReleaseObjects()
{
	mGraphicsRootSignature = nullptr;
	MeshRenderer::Release();
}


void Scene::CreateShaderResourceView(RComPtr<ID3D12Resource> resource, DXGI_FORMAT dxgiSrvFormat)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = dxgiSrvFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	mDescriptorHeap->CreateSrv(resource, &srvDesc);
}


void Scene::CreateShaderResourceView(Texture* texture, UINT descriptorHeapIndex)
{
	ComPtr<ID3D12Resource> resource = texture->GetResource();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = texture->GetShaderResourceViewDesc();

	mDescriptorHeap->CreateSrvs(resource, &srvDesc, descriptorHeapIndex);
	texture->SetGpuDescriptorHandle(mDescriptorHeap->GetGPUSrvLastHandle());
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////* Render *//////////////////

void Scene::OnPrepareRender()
{
	cmdList->SetGraphicsRootSignature(GetRootSignature().Get());

	mainCamera->SetViewportsAndScissorRects();

	mDescriptorHeap->Set();

	UpdateShaderVariables();

	// Game Info
	static float timeElapsed{};
	timeElapsed += DeltaTime();
	crntScene->SetGraphicsRoot32BitConstants(RootParam::GameInfo, timeElapsed, 0);
}


void Scene::RenderTerrain()
{
	if (mTerrain) {
		mTerrain->Render();
	}
}


void Scene::RenderGridObjects(std::set<GameObject*>& renderObjects, std::set<GameObject*>& transparentObjects, std::set<GameObject*>& billboardObjects)
{
	for (const auto& grid : mGrids) {
		if (grid.Empty()) {
			continue;
		}

		if (mainCamera->IsInFrustum(grid.GetBB())) {
			auto& objects = grid.GetObjects();
			renderObjects.insert(objects.begin(), objects.end());
		}
	}

	for (auto& object : renderObjects) {
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


void Scene::RenderBounds(const std::set<GameObject*>& renderObjects)
{
	for (auto& player : mPlayers) {
		if (player->IsActive()) {
			player->RenderBounds();
		}
	}

	for (auto& object : renderObjects) {
		object->RenderBounds();
	}
}


void UpdateGridShaderVariables(const Grid& grid)
{

	BoundingBox box = grid.GetBB();

	Matrix transform = XMMatrixIdentity();

#ifdef DRAW_SCENE_GRID_3D
	XMMatrix::SetPosition(transform, Vec3(box.Center.x, box.Center.y, box.Center.z));
#else
	constexpr float meshHeight = 30.0f;
	XMMatrix::SetPosition(transform, Vec3(box.Center.x, 30.0f, box.Center.z));
#endif

	crntScene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrixTranspose(transform), 0);
}

void Scene::RenderGridBounds()
{
	for (Grid& grid : mGrids) {
		UpdateGridShaderVariables(grid);
		mGridMesh->Render();
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

static D3D12_PRIMITIVE_TOPOLOGY objectPrimitiveTopology  = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
static D3D12_PRIMITIVE_TOPOLOGY terrainPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
static D3D12_PRIMITIVE_TOPOLOGY boundsPrimitiveTopology  = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
void Scene::Render()
{
	sptr<Camera> camera = mMainCamera->GetCamera();
	std::set<GameObject*> renderObjects{};
	std::set<GameObject*> transparentObjects{};
	std::set<GameObject*> billboardObjects{};

	OnPrepareRender();

	// objects
	cmdList->IASetPrimitiveTopology(objectPrimitiveTopology);

	mGlobalShader->Render();

	RenderGridObjects(renderObjects, transparentObjects, billboardObjects);
	RenderBackgrounds();
	RenderBullets();

	RenderInstanceObjects();

	if (mSmallExpFXShader) {
		mSmallExpFXShader->Render();
	}
	if (mBigExpFXShader) {
		mBigExpFXShader->Render();
	}

	RenderBillboards(billboardObjects);

	// transparent
	mTransparentShader->Render();
	for (auto& object : transparentObjects) {
		object->Render();
	}
	if (mWater) {
		mWaterShader->Render();
		mWater->Render();
	}
	
	// canvas
	Canvas::Inst()->Render();

	// terrain
	cmdList->IASetPrimitiveTopology(terrainPrimitiveTopology);
	RenderTerrain();

	cmdList->IASetPrimitiveTopology(objectPrimitiveTopology);

	// skybox
	mSkyBox->Render();

	// bounds
	if (mIsRenderBounds && mBoundingShader) {
		cmdList->IASetPrimitiveTopology(boundsPrimitiveTopology);
		mBoundingShader->Render();

		RenderBounds(renderObjects);
		RenderGridBounds();
	}

	
}

bool IsBehind(const Vec3& point, const Vec4& plane)
{
	return XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&plane), _VECTOR(point))) < 0.f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////* Update *//////////////////

void Scene::UpdateLights()
{
	
}


void Scene::UpdateObjects()
{
	ProcessObjects([this](sptr<GameObject> object) {
		UpdateObject(object.get());
		});

	mTerrain->Update();

	if (mSmallExpFXShader) {
		mSmallExpFXShader->Update();
	}

	if (mBigExpFXShader) {
		mBigExpFXShader->Update();
	}

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

	Canvas::Inst()->Update();
}


void Scene::UpdateCamera()
{
	mMainCamera->Update();
}


void Scene::AnimateObjects()
{
	/*ProcessObjects([](sptr<GameObject> object) {
		object->Animate();
		});*/
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


void Scene::Start()
{
	ProcessObjects([](sptr<GameObject> object) {
		object->Start();
		});

	mTerrain->Start();
	mMainCamera->Start();
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

	AnimateObjects();
	UpdateLights();

	UpdateCamera();
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////* Others *//////////////////
void Scene::CreateSpriteEffect(Vec3 pos, float speed, float scale)
{
	sptr<GameObject> effect = std::make_shared<GameObject>();
	effect->SetModel(GetModel("sprite_explosion"));
	effect->RemoveComponent<ObjectCollider>();
	auto& script = effect->AddComponent<Script_Sprite>();

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

	int gridX = static_cast<int>(pos.x / mGridLength);
	int gridZ = static_cast<int>(pos.z / mGridLength);

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


void Scene::ProcessInput(HWND hWnd, POINT oldCursorPos)
{

}


bool Scene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}


bool Scene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_OEM_6:
			ChangeToNextPlayer();
			break;
		case VK_OEM_4:
			ChangeToPrevPlayer();
			break;

		case VK_F1:
			break;
		case VK_F2:
			break;
		case VK_F5:
			ToggleDrawBoundings();
			break;
		case VK_F6:
			mLight->SetGlobalLight(GlobalLight::Sunlight);
			break;
		case VK_F7:
			mLight->SetGlobalLight(GlobalLight::MoonLight);
			break;
		default:
			break;
		}
	}

	break;
	default:
		break;
	}

	

	return true;
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
	++mCrntPlayerIndex;
	if (mCrntPlayerIndex >= mPlayers.size()) {
		mCrntPlayerIndex = 0;
	}

	mPlayer = mPlayers[mCrntPlayerIndex];
}


void Scene::ChangeToPrevPlayer()
{
	--mCrntPlayerIndex;
	if (mCrntPlayerIndex < 0) {
		mCrntPlayerIndex = static_cast<int>(mPlayers.size() - 1);
	}

	mPlayer = mPlayers[mCrntPlayerIndex];
}

// 객체의 Grid정보를 업데이트한다.
void Scene::UpdateObjectGrid(GameObject* object, bool isCheckAdj)
{
	if (!object->IsActive()) {
		return;
	}

	int gridIndex = GetGridIndexFromPos(object->GetPosition());

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

	for (auto& object : mStatiObjects) {
		processFunc(object);
	}

	for (auto& object : mExplosiveObjects) {
		processFunc(object);
	}
}