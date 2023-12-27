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


// [ CScene ] //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////

static uptr<CScene> sceneInstance{};

void CScene::Create()
{
	sceneInstance = std::make_unique<CScene>();
}


void CScene::Destroy()
{
	sceneInstance = nullptr;
	Canvas::Inst()->OnDestroy();
}


CScene* CScene::Inst()
{
	return sceneInstance.get();
}



CScene::CScene()
{
	constexpr int gridLengthCount{ 20 };		// gridCount = n*n

	constexpr Vec3 borderPos = Vec3(256, 200, 256);
	constexpr Vec3 borderExtents = Vec3(1500, 500, 1500);

	mMapBorder = { borderPos, borderExtents };	// map segmentation criteria
	mGridLength = static_cast<int>(mMapBorder.Extents.x / gridLengthCount);
	mLight = std::make_unique<CLight>();

	mDescriptorHeap = std::make_shared<CDescriptorHeap>();
}


CScene::~CScene()
{

}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////
float CScene::GetTerrainHeight(float x, float z) const
{
	return (mTerrain) ? mTerrain->GetHeight(x, z) : 0.0f;
}


rsptr<const CMasterModel> CScene::GetModel(const std::string& modelName)
{
	assert(mModels.contains(modelName));

	return mModels[modelName];
}


rsptr<CTexture> CScene::GetTexture(const std::string& name)
{
	assert(mMaterialMap.contains(name)); return mMaterialMap[name]->mTexture;
}

rsptr<Camera> CScene::GetMainCamera() const
{
	return mMainCamera->GetCamera();
}

sptr<CCameraObject> CScene::GetCameraObject() const
{
	return mMainCamera;
}

RComPtr<ID3D12RootSignature> CScene::GetRootSignature() const
{
	return mGraphicsRootSignature->Get();
}

UINT CScene::GetRootParamIndex(RootParam param)
{
	return mGraphicsRootSignature->GetRootParamIndex(param);
}

void CScene::SetGlobalShader() const
{
	mGlobalShader->Render();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////* DirectX *//////////////////
void CScene::CreateGraphicsRootSignature()
{
	mGraphicsRootSignature = std::make_shared<CGraphicsRootSignature>();

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


void CScene::CreateShaderVariables()
{
	mLight->CreateShaderVariables();
}


void CScene::UpdateShaderVariables()
{
	mLight->UpdateShaderVariables();
	mMainCamera->UpdateShaderVariables();
}


void CScene::ReleaseShaderVariables()
{
	mLight->ReleaseShaderVariables();
	mMainCamera->ReleaseShaderVariables();
}


void CScene::ReleaseUploadBuffers()
{
	ProcessObjects([](sptr<CGameObject> object) {
		object->ReleaseUploadBuffers();
		});

	MeshRenderer::ReleaseStaticUploadBuffers();
}

void CScene::SetGraphicsRoot32BitConstants(RootParam param, const Matrix& data, UINT offset)
{
	constexpr UINT num32Bit = 16;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}

void CScene::SetGraphicsRoot32BitConstants(RootParam param, const Vec4x4& data, UINT offset)
{
	constexpr UINT num32Bit = 16;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}

void CScene::SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset)
{
	constexpr UINT num32Bit = 4;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}

void CScene::SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset)
{
	constexpr UINT num32Bit = 1;
	cmdList->SetGraphicsRoot32BitConstants(GetRootParamIndex(param), num32Bit, &data, offset);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////* Build *//////////////////
void CScene::BuildShaders()
{
	BuildGlobalShader();
	BuildBoundingShader();
	BuildSmallExpFXShader();
	BuildBigExpFXShader();
	BuildBillboardShader();
}

void CScene::BuildGlobalShader()
{
	mGlobalShader = std::make_shared<CTexturedShader>();
	mGlobalShader->CreateShader();
	
	mWaterShader = std::make_shared<CWaterShader>();
	mWaterShader->CreateShader();

	mInstancingShader = std::make_shared<CObjectInstancingShader>();
	mInstancingShader->CreateShader();

	mTransparentShader = std::make_shared<CTransparentShader>();
	mTransparentShader->CreateShader();
}


void CScene::BuildBoundingShader()
{
	mBoundingShader = std::make_shared<CWireShader>();
	mBoundingShader->CreateShader();
}


void CScene::BuildSmallExpFXShader()
{
	mSmallExpFXShader = std::make_shared<CSmallExpEffectShader>();
	mSmallExpFXShader->Create();
}


void CScene::BuildBigExpFXShader()
{
	mBigExpFXShader = std::make_shared<CBigExpEffectShader>();
	mBigExpFXShader->Create();
}

void CScene::BuildBillboardShader()
{
	mBillboardShader = std::make_shared<CBillboardShader>();
	mBillboardShader->CreateShader();
	
	mSpriteShader = std::make_shared<CSpriteShader>();
	mSpriteShader->CreateShader();
}


void CScene::BuildPlayers()
{
	LIGHT_RANGE lightRange{};
	// [1]
	mPlayers.reserve(2);
	sptr<CGameObject> airplanePlayer = std::make_shared<CGameObject>();
	airplanePlayer->AddComponent<Script_AirplanePlayer>()->CreateBullets(GetModel("tank_bullet"));
	airplanePlayer->SetModel(GetModel("Gunship"));

	mPlayers.emplace_back(airplanePlayer);


	//// [2]
	//sptr<CGameObject> tankPlayer = std::make_shared<CGameObject>();
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


void CScene::BuildTerrain()
{
	constexpr int terrainGridLength = (TERRAIN_LENGTH - 1) / 8 + 1; // (512 / 8) = 64, 64 + 1 = 65

	mTerrain = std::make_shared<CHeightMapTerrain>(_T("HeightMap.raw"), TERRAIN_LENGTH, TERRAIN_LENGTH, terrainGridLength, terrainGridLength);

	BuildGrid();
}


void CScene::BuildGrid()
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


void CScene::BuildGridObjects()
{
	ProcessObjects([this](sptr<CGameObject> object) {
		UpdateObjectGrid(object.get());
		});

	for (auto& terrain : mTerrain->GetTerrains()) {
		UpdateObjectGrid(terrain.get());
	}
}


void CScene::BuildCamera()
{
	mMainCamera = std::make_shared<CMainCamera>();
	mMainCamera->CreateShaderVariables();
}


void CScene::LoadModels()
{
	const std::vector<std::string> binModelNames = { "tank_bullet", "sprite_explosion", };

	sptr<CMasterModel> model;
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


void CScene::InitObjectByTag(const void* pTag, sptr<CGameObject> object)
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
	case ObjectTag::Mirror:
	{
		mMirrorShader->SetMirrorObject(object);
		return;
	}
	break;
	default:
		break;
	}

	mStaticObjects.emplace_back(object);
	object->SetFlyable(true);
}

void CScene::LoadGameObjects(FILE* file)
{
	std::string token{};
	std::string name{};

	UINT nReads = 0;

	int objectCount;
	::ReadUnityBinaryString(file, token); // "<GameObjects>:"
	nReads = (UINT)::fread(&objectCount, sizeof(int), 1, file);

	mStaticObjects.reserve(objectCount);

	int sameObjectCount{};			// get one unique model from same object
	sptr<CMasterModel> model{};
	sptr<CObjectInstanceBuffer> instBuffer{};
	bool isInstancing{};
	ObjectTag tag{};
	ObjectLayer layer{};

	for (int i = 0; i < objectCount; ++i) {
		sptr<CGameObject> object{};

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
				instBuffer = std::make_shared<CObjectInstanceBuffer>();
				instBuffer->CreateShaderVariables(sameObjectCount);
				instBuffer->SetModel(model);
				if (GetObjectType(tag) == ObjectType::Dynamic) {
					instBuffer->SetDynamic();
				}
				mInstanceBuffers.emplace_back(instBuffer);
			}
		}

		if (isInstancing) {
			object = std::make_shared<CInstancingObject>();
		}
		else {
			object = std::make_shared<CGameObject>();
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
			((CInstancingObject*)object.get())->SetBuffer(instBuffer);
		}

		--sameObjectCount;
	}
}





void CScene::LoadSceneObjectsFromFile(const std::string& fileName)
{
	FILE* file = NULL;
	::fopen_s(&file, fileName.c_str(), "rb");
	assert(file);
	::rewind(file);

	mLight->BuildLights(file);
	LoadGameObjects(file);
}

void CScene::LoadTextures()
{
	std::vector<std::string> textureNames;
	
	::LoadTextureNames(textureNames, "Models/Textures");

	MATERIALLOADINFO info{}; 
	info.mAlbedo = Vec4(0.1f, .1f, .1f, 1.0f);
	for (auto& textureName : textureNames) {
		// load texture
		sptr<CTexture> texture = std::make_shared<CTexture>(RESOURCE_TEXTURE2D);
		texture->LoadTexture(textureName);

		// apply to material
		sptr<CMaterial> material = std::make_shared<CMaterial>();
		material->SetTexture(texture);

		sptr<CMaterialColors> materialColors = std::make_shared<CMaterialColors>(info);
		material->SetMaterialColors(materialColors);

		mMaterialMap.insert(std::make_pair(textureName, material));
	}
}


void CScene::CreateCbvSrvDescriptorHeaps(int cbvCount, int srvCount)
{
	mDescriptorHeap->Create(cbvCount, srvCount);
}


void CScene::BuildObjects()
{
	CreateGraphicsRootSignature();

	assert(mGraphicsRootSignature);
	CreateCbvSrvDescriptorHeaps(0, 1024);

	// load textures
	LoadTextures();

	// load canvas (UI)
	Canvas::Inst()->Create();

	// load models
	mMirrorShader = std::make_shared<CMirrorShader>();
	mMirrorShader->CreateShader();
	LoadSceneObjectsFromFile("Models/Scene.bin");
	LoadModels();

	// build settings
	BuildPlayers();
	BuildTerrain();

	// shader variables
	BuildShaders();
	CreateShaderVariables();

#ifdef DRAW_SCENE_GRID_3D
	mGridMesh = std::make_shared<CModelObjectMesh>((float)mGridLength, mMaxGridHeight, (float)mGridLength, false, true);
#else
	mGridMesh = std::make_shared<CModelObjectMesh>((float)mGridLength, (float)mGridLength, true);
#endif

	MeshRenderer::BuildMeshes();

	// camera
	BuildCamera();

	// skybox
	mSkyBox = std::make_shared<CSkyBox>();
}


void CScene::ReleaseObjects()
{
	mGraphicsRootSignature = nullptr;
	MeshRenderer::Release();
}


void CScene::CreateShaderResourceView(RComPtr<ID3D12Resource> resource, DXGI_FORMAT dxgiSrvFormat)
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


void CScene::CreateShaderResourceView(CTexture* texture, UINT descriptorHeapIndex)
{
	ComPtr<ID3D12Resource> resource = texture->GetResource();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = texture->GetShaderResourceViewDesc();

	mDescriptorHeap->CreateSrvs(resource, &srvDesc, descriptorHeapIndex);
	texture->SetGpuDescriptorHandle(mDescriptorHeap->GetGPUSrvLastHandle());
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////* Render *//////////////////

void CScene::OnPrepareRender()
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


void CScene::RenderTerrain(bool isMirror)
{
	if (mTerrain) {
		mTerrain->Render(isMirror);
	}
}


void CScene::RenderGridObjects(std::set<CGameObject*>& renderObjects, std::set<CGameObject*>& transparentObjects, std::set<CGameObject*>& billboardObjects)
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
			if (!object->IsInstancing()) {
				object->Render();
			}
			break;
		}
	}
}

void CScene::RenderInstanceObjects()
{
	mInstancingShader->Render();
	for (auto& buffer : mInstanceBuffers) {
		buffer->Render();
	}
}

void CScene::RenderBackgrounds()
{
	for (auto& background : mBackgrounds) {
		background->Render();
	}
}


void CScene::RenderBullets()
{
	for (auto& player : mPlayers) {
		if (player->IsActive()) {
			player->GetComponent<Script_AirplanePlayer>()->RenderBullets();
		}
	}
}


void CScene::RenderBounds(const std::set<CGameObject*>& renderObjects)
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


void UpdateGridShaderVariables(const CGrid& grid)
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

void CScene::RenderGridBounds()
{
	for (CGrid& grid : mGrids) {
		UpdateGridShaderVariables(grid);
		mGridMesh->Render();
	}
}

void CScene::RenderBillboards(const std::set<CGameObject*>& billboards)
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
void CScene::Render()
{
	sptr<Camera> camera = mMainCamera->GetCamera();
	std::set<CGameObject*> renderObjects{};
	std::set<CGameObject*> transparentObjects{};
	std::set<CGameObject*> billboardObjects{};

	OnPrepareRender();

	// objects
	cmdList->IASetPrimitiveTopology(objectPrimitiveTopology);

	// mirror
	RenderMirror();

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

void CScene::RenderMirrorObjects(const Vec4& mirrorPlane)
{
	std::set<CGameObject*> renderObjects;
	std::set<CGameObject*> billboardObjects;
	for (const auto& grid : mGrids) {
		if (grid.Empty()) {
			continue;
		}

		std::vector<Vec3> corners(8);
		grid.GetBB().GetCorners(corners.data());
		bool isGridFront{ false };
		for (const Vec3& point : corners) {
			if (!IsBehind(point, mirrorPlane)) {
				isGridFront = true;
				break;
			}
		}
		if (!isGridFront) {
			continue;
		}

		auto& objects = grid.GetObjects();
		renderObjects.insert(objects.begin(), objects.end());
	}

	for (auto& object : renderObjects) {

		if (object->IsTransparent()) {
			continue;
		}

		if (object->GetTag() != ObjectTag::Terrain && IsBehind(object->GetPosition(), mirrorPlane)) {
			continue;
		}

		switch (object->GetTag())
		{
		case ObjectTag::Bullet:
			break;
		case ObjectTag::Billboard:
		case ObjectTag::Sprite:
			break;
		default:
			if (!object->IsInstancing()) {
				object->Render();
			}
			break;
		}
	}

	RenderBackgrounds();

	cmdList->IASetPrimitiveTopology(terrainPrimitiveTopology);
	RenderTerrain(true);

	cmdList->IASetPrimitiveTopology(objectPrimitiveTopology);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////* Update *//////////////////

void CScene::UpdateLights()
{
	
}


void CScene::UpdateObjects()
{
	ProcessObjects([this](sptr<CGameObject> object) {
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


void CScene::UpdateCamera()
{
	mMainCamera->Update();
}


void CScene::AnimateObjects()
{
	/*ProcessObjects([](sptr<CGameObject> object) {
		object->Animate();
		});*/
}


void CScene::CheckCollisions()
{
	UpdatePlayerGrid();

	for (CGrid& grid : mGrids) {
		grid.CheckCollisions();
	}

	DeleteExplodedObjects();
}


void CScene::UpdatePlayerGrid()
{
	for (auto& player : mPlayers) {
		UpdateObjectGrid(player.get());

		const std::list<sptr<CGameObject>>* bullets = player->GetComponent<Script_AirplanePlayer>()->GetBullets();

		if (!bullets) {
			continue;
		}

		for (auto& bullet : *bullets) {
			UpdateObjectGrid(bullet.get(), true);
		}
	}
}


void CScene::UpdateObject(CGameObject* object)
{
	if (!object) {
		return;
	}

	object->Update();

	if (object->GetType() == ObjectType::DynamicMove) {
		UpdateObjectGrid(object);
	}
}


void CScene::Start()
{
	ProcessObjects([](sptr<CGameObject> object) {
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
void CScene::Update()
{
	CheckCollisions();

	UpdateObjects();

	AnimateObjects();
	UpdateLights();

	UpdateCamera();
}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////* Others *//////////////////
void CScene::CreateSpriteEffect(Vec3 pos, float speed, float scale)
{
	sptr<CGameObject> effect = std::make_shared<CGameObject>();
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

void CScene::CreateSmallExpFX(Vec3 pos)
{
	if (mSmallExpFXShader) {
		mSmallExpFXShader->SetActive(pos);
	}

	CreateSpriteEffect(pos, 0.0001f);
}


void CScene::CreateBigExpFX(Vec3 pos)
{
	if (mBigExpFXShader) {
		mBigExpFXShader->SetActive(pos);
	}

	CreateSpriteEffect(pos, 0.025f, 5.f);
}


int CScene::GetGridIndexFromPos(Vec3 pos)
{
	pos.x -= mGridStartPoint;
	pos.z -= mGridStartPoint;

	int gridX = static_cast<int>(pos.x / mGridLength);
	int gridZ = static_cast<int>(pos.z / mGridLength);

	return gridZ * mGridCols + gridX;
}


void CScene::SetObjectGridIndex(rsptr<CGameObject> object, int gridIndex)
{
	object->SetGridIndex(gridIndex);

	constexpr int gridRange = 1;
	std::unordered_set<int> gridIndices{};
	gridIndices.insert(gridIndex);

	object->SetGridIndices(gridIndices);
}


void CScene::DeleteExplodedObjects()
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


void CScene::ProcessInput(HWND hWnd, POINT oldCursorPos)
{

}


bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}


bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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


void CScene::ToggleDrawBoundings()
{
	mIsRenderBounds = !mIsRenderBounds;

	ProcessObjects([](sptr<CGameObject> object) {
		object->ToggleDrawBoundings();
		});
}


void CScene::CreateExplosion(ExplosionType explosionType, const Vec3& pos)
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


void CScene::BlowAllExplosiveObjects()
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


void CScene::ChangeToNextPlayer()
{
	++mCrntPlayerIndex;
	if (mCrntPlayerIndex >= mPlayers.size()) {
		mCrntPlayerIndex = 0;
	}

	mPlayer = mPlayers[mCrntPlayerIndex];
}


void CScene::ChangeToPrevPlayer()
{
	--mCrntPlayerIndex;
	if (mCrntPlayerIndex < 0) {
		mCrntPlayerIndex = static_cast<int>(mPlayers.size() - 1);
	}

	mPlayer = mPlayers[mCrntPlayerIndex];
}

// 객체의 Grid정보를 업데이트한다.
void CScene::UpdateObjectGrid(CGameObject* object, bool isCheckAdj)
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

	std::unordered_set<int> gridIndices{};

	// 1칸 이내의 인접한 그리드 충돌검사
	// BoundingSphere가 Grid 내부에 완전히 포함되면 검사 X
	const auto& objectBS = object->GetComponent<ObjectCollider>()->GetBS();
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


void CScene::RemoveObjectFromGrid(CGameObject* object)
{
	for (int index : object->GetGridIndices()) {
		mGrids[index].RemoveObject(object);
	}

	object->ClearGridIndices();
}


void CScene::ProcessObjects(std::function<void(sptr<CGameObject>)> processFunc)
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

void CScene::RenderMirror()
{

}

const XMMATRIX& CScene::GetReflect() const
{
	return mMirrorShader->GetReflect();
}

bool CScene::IsRenderReflectObject()
{
	return mMirrorShader->IsRenderReflectObject();
}
