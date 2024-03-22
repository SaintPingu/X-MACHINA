#pragma region Include
#include "stdafx.h"
#include "Scene.h"
#include "DXGIMgr.h"
#include "MultipleRenderTarget.h"
#include "FrameResource.h"

#include "ResourceMgr.h"
#include "UI.h"
#include "Object.h"
#include "Model.h"
#include "Terrain.h"
#include "Shader.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "Timer.h"
#include "FileIO.h"
#include "Light.h"
#include "Collider.h"
#include "SkyBox.h"
#include "Texture.h"
#include "ObjectPool.h"

#include "Script_Player.h"
#include "Script_ExplosiveObject.h"
#include "Script_Billboard.h"
#include "Script_Sprite.h"

#include "TestCube.h"
#include "Ssao.h"
#include "ParticleSystem.h"
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region C/Dtor
namespace {
	constexpr int kGridWidthCount = 20;						// all grid count = n*n
	constexpr Vec3 kBorderPos = Vec3(256, 200, 256);		// center of border
	constexpr Vec3 kBorderExtents = Vec3(1500, 500, 1500);		// extents of border
}

Scene::Scene()
	:
	mMapBorder(kBorderPos, kBorderExtents),
	mGridhWidth(static_cast<int>(mMapBorder.Extents.x / kGridWidthCount)),
	mLight(std::make_shared<Light>())
{

}

void Scene::Release()
{
	mainCameraObject->Destroy();
	canvas->Release();

	Destroy();
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Getter
float Scene::GetTerrainHeight(float x, float z) const
{
	assert(mTerrain);

	return mTerrain->GetHeight(x, z);
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

void Scene::UpdateShaderVars()
{
	UpdateMainPassCB();
	UpdateShadowPassCB();
	UpdateSsaoCB();
	UpdateMaterialBuffer();
}

void Scene::UpdateMainPassCB()
{
	PassConstants passCB;
	passCB.MtxView						= mainCamera->GetViewMtx().Transpose();
	passCB.MtxProj						= mainCamera->GetProjMtx().Transpose();
	passCB.MtxShadow					= mLight->GetShadowMtx().Transpose();
	passCB.CameraPos					= mainCamera->GetPosition();
	passCB.CameraRight					= mainCamera->GetRight();
	passCB.CameraUp						= mainCamera->GetUp();
	passCB.DeltaTime					= DeltaTime();
	passCB.TotalTime					= timer->GetTotalTime();
	passCB.FrameBufferWidth				= gkFrameBufferWidth;
	passCB.FrameBufferHeight			= gkFrameBufferHeight;
	passCB.SkyBoxIndex					= mSkyBox->GetTexture()->GetSrvIdx();
	passCB.DefaultDsIndex				= res->Get<Texture>("DefaultDepthStencil")->GetSrvIdx();
	passCB.ShadowDsIndex				= res->Get<Texture>("ShadowDepthStencil")->GetSrvIdx();
	passCB.RT0G_PositionIndex			= res->Get<Texture>("PositionTarget")->GetSrvIdx();
	passCB.RT1G_NormalIndex				= res->Get<Texture>("NormalTarget")->GetSrvIdx();
	passCB.RT2G_DiffuseIndex			= res->Get<Texture>("DiffuseTarget")->GetSrvIdx();
	passCB.RT3G_EmissiveIndex			= res->Get<Texture>("EmissiveTarget")->GetSrvIdx();
	passCB.RT4G_MetallicSmoothnessIndex = res->Get<Texture>("MetallicSmoothnessTarget")->GetSrvIdx();
	passCB.RT5G_OcclusionIndex			= res->Get<Texture>("OcclusionTarget")->GetSrvIdx();
	passCB.RT0L_DiffuseIndex			= res->Get<Texture>("DiffuseAlbedoTarget")->GetSrvIdx();
	passCB.RT1L_SpecularIndex			= res->Get<Texture>("SpecularAlbedoTarget")->GetSrvIdx();
	passCB.RT2L_AmbientIndex			= res->Get<Texture>("AmbientTarget")->GetSrvIdx();
	passCB.RT0S_SsaoIndex				= res->Get<Texture>("SSAOTarget_0")->GetSrvIdx();
	passCB.LightCount					= mLight->GetLightCount();
	passCB.GlobalAmbient				= Vec4(0.4f, 0.4f, 0.4f, 1.f);
	passCB.FilterOption					= dxgi->GetFilterOption();
	passCB.ShadowIntensity				= 0.0f;
	passCB.FogColor						= Colors::Gray;
	memcpy(&passCB.Lights, mLight->GetSceneLights().get()->Lights.data(), sizeof(passCB.Lights));
	
	frmResMgr->CopyData(0, passCB);
}

void Scene::UpdateShadowPassCB()
{
	PassConstants passCB;
	passCB.MtxView = mLight->GetLightViewMtx().Transpose();
	passCB.MtxProj = mLight->GetLightProjMtx().Transpose();

	frmResMgr->CopyData(1, passCB);
}

void Scene::UpdateSsaoCB()
{
	SsaoConstants ssaoCB;

	Matrix mtxProj = mainCamera->GetProjMtx();
	Matrix mtxTex = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f };

	ssaoCB.MtxInvProj = mainCamera->GetProjMtx().Invert().Transpose();
	ssaoCB.MtxProjTex = (mtxProj * mtxTex).Transpose();
	dxgi->GetSsao()->GetOffsetVectors(ssaoCB.OffsetVectors);
	
	// for Blur 
	auto blurWeights = Filter::CalcGaussWeights(2.5f);
	ssaoCB.BlurWeights[0] = Vec4(&blurWeights[0]);
	ssaoCB.BlurWeights[1] = Vec4(&blurWeights[4]);
	ssaoCB.BlurWeights[2] = Vec4(&blurWeights[8]);

	auto ssaoTarget = res->Get<Texture>("SSAOTarget_0");
	ssaoCB.InvRenderTargetSize = Vec2{ 1.f / ssaoTarget->GetWidth(), 1.f / ssaoTarget->GetHeight() };

	// coordinates given in view space.
	ssaoCB.OcclusionRadius = 0.5f;
	ssaoCB.OcclusionFadeStart = 0.2f;
	ssaoCB.OcclusionFadeEnd = 1.0f;
	ssaoCB.SurfaceEpsilon = 0.05f;
	ssaoCB.AccessContrast = 12;

	ssaoCB.RandomVectorIndex = res->Get<Texture>("RandomVector")->GetSrvIdx();

	frmResMgr->CopyData(ssaoCB);
}

void Scene::UpdateMaterialBuffer()
{
	res->ProcessFunc<MasterModel>(
		[](sptr<MasterModel> model) {
			model->GetMesh()->UpdateMaterialBuffer(); 
		});
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Build
void Scene::BuildObjects()
{
	// load canvas (UI)
	canvas->Init();

	// load models
	LoadSceneObjects("Import/Scene.bin");

	// build settings
	BuildPlayers();
	BuildTerrain();
	BuildTest();

	// build static meshes
	MeshRenderer::BuildMeshes();

	// skybox
	mSkyBox = std::make_shared<SkyBox>();
}

void Scene::ReleaseObjects()
{
	MeshRenderer::Release();
}

void Scene::BuildPlayers()
{
	mPlayers.reserve(1);
	sptr<GridObject> airplanePlayer = std::make_shared<GridObject>();
	airplanePlayer->AddComponent<Script_GroundPlayer>()->CreateBullets(res->Get<MasterModel>("tank_bullet"));
	//airplanePlayer->AddComponent<Script_AirplanePlayer>()->CreateBullets(GetModel("tank_bullet"));
	airplanePlayer->SetModel(res->Get<MasterModel>("EliteTrooper"));
	
	mPlayers.push_back(airplanePlayer);
	mPlayer = mPlayers.front();
	mPlayer->AddComponent<ParticleSystem>()->Load("Green")->SetTarget("Humanoid_ R Hand");
	mPlayer->AddComponent<ParticleSystem>()->Load("Fire")->SetTarget("Humanoid_ L Hand");
	mPlayer->AddComponent<ParticleSystem>()->Load("Fountain")->SetTarget("Humanoid_ Head");
}

void Scene::BuildTerrain()
{
	mTerrain = std::make_shared<Terrain>("Import/Terrain.bin");

	BuildGrid();
}

void Scene::BuildTest()
{
	mTestCubes.resize(2);
	mTestCubes[0] = std::make_shared<TestCube>(Vec2(170, 150));
	mTestCubes[0]->GetMaterial()->SetMatallic(0.f);
	mTestCubes[0]->GetMaterial()->SetRoughness(0.f);
	mTestCubes[0]->GetMaterial()->SetTexture(TextureMap::DiffuseMap0, res->Get<Texture>("Rock_BaseColor"));
	mTestCubes[0]->GetMaterial()->SetTexture(TextureMap::NormalMap, res->Get<Texture>("Rock_Normal"));

	mTestCubes[1] = std::make_shared<TestCube>(Vec2(165, 150));
	mTestCubes[1]->GetMaterial()->SetMatallic(0.f);
	mTestCubes[1]->GetMaterial()->SetRoughness(0.f);
	mTestCubes[1]->GetMaterial()->SetTexture(TextureMap::DiffuseMap0, res->Get<Texture>("Wall_BaseColor"));
	mTestCubes[1]->GetMaterial()->SetTexture(TextureMap::NormalMap, res->Get<Texture>("Wall_Normal"));

	mPlayer = mPlayers.front();
	mPlayerScript = mPlayer->GetComponent<Script_GroundPlayer>();

	mParticles.resize(3);
#pragma region light
	mParticles[0] = std::make_shared<GameObject>();
	mParticles[0]->SetPosition(Vec3{ 166.f, 10.5f, 149.f });
	mParticles[0]->AddComponent<ParticleSystem>()->Load("Light");
#pragma endregion
	// HeightMap_1024x1024_R32
#pragma region MagicMissile
	mParticles[1] = std::make_shared<GameObject>();
	mParticles[1]->SetPosition(Vec3{ 167.5f, 11.f, 150.f });
	mParticles[1]->AddComponent<ParticleSystem>()->Load("Small_MagicMissile_Out");
	mParticles[1]->AddComponent<ParticleSystem>()->Load("Big_MagicMissile_Out");
	mParticles[1]->AddComponent<ParticleSystem>()->Load("Big_MagicMissile_Light");
	mParticles[1]->AddComponent<ParticleSystem>()->Load("Small_MagicMissile_Light");
#pragma endregion

#pragma region ShapeTest
	mParticles[2] = std::make_shared<GameObject>();
	mParticles[2]->SetPosition(Vec3{ 173.f, 11.f, 150.f });
	auto& component = mParticles[2]->AddComponent<ParticleSystem>()->Load("Green");
	component->GetPSCD().StartLifeTime = 0.3f;
	component->GetPSCD().Emission.SetBurst(100);
	component->GetPSCD().Duration = 0.2f;
	component->GetPSCD().Shape.SetSphere(1.5f, 0.f, 360.f, true);
#pragma endregion
}

void Scene::BuildGrid()
{
	constexpr float kMaxHeight = 300.f;	// for 3D grid

	// recalculate scene grid size
	const int adjusted = Math::GetNearestMultiple((int)mMapBorder.Extents.x, mGridhWidth);
	mMapBorder.Extents = Vec3((float)adjusted, mMapBorder.Extents.y, (float)adjusted);

	// set grid start pos
	mGridStartPoint = mMapBorder.Center.x - mMapBorder.Extents.x / 2;

	// set grid count
	mGridCols = adjusted / mGridhWidth;
	const int gridCount = mGridCols * mGridCols;
	mGrids.resize(gridCount);

	// set grid bounds
	const float gridExtent = (float)mGridhWidth / 2.0f;
	for (int y = 0; y < mGridCols; ++y) {
		for (int x = 0; x < mGridCols; ++x) {
			float gridX = (mGridhWidth * x) + ((float)mGridhWidth / 2) + mGridStartPoint;
			float gridZ = (mGridhWidth * y) + ((float)mGridhWidth / 2) + mGridStartPoint;

			BoundingBox bb{};
			bb.Center = Vec3(gridX, kMaxHeight / 2, gridZ);
			bb.Extents = Vec3(gridExtent, kMaxHeight, gridExtent);

			int index = (y * mGridCols) + x;
			mGrids[index].Init(index, bb);
		}
	}
}

void Scene::UpdateGridInfo()
{
	ProcessObjects([this](sptr<GridObject> object) {
		UpdateObjectGrid(object.get());
		});

	mTerrain->UpdateGrid();
}


void Scene::LoadSceneObjects(const std::string& fileName)
{
	FILE* file = nullptr;
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
	sptr<ObjectPool> objectPool{};

	bool isInstancing{};
	ObjectTag tag{};
	ObjectLayer layer{};

	for (int i = 0; i < objectCount; ++i) {
		sptr<GridObject> object{};

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
			model = res->Get<MasterModel>(meshName);

			FileIO::ReadString(file, token); //"<Transforms>:"
			FileIO::ReadVal(file, sameObjectCount);

			FileIO::ReadString(file, token); //"<IsInstancing>:"
			FileIO::ReadVal(file, isInstancing);

			if (isInstancing) {
				objectPool = std::make_shared<ObjectPool>(model, sameObjectCount, sizeof(SB_StandardInst));
				objectPool->CreateObjects<InstObject>();
				mObjectPools.emplace_back(objectPool);
			}
		}

		if (isInstancing) {
			// 인스턴싱 객체는 생성된 객체를 받아온다.
			object = objectPool->Get(false);
		}
		else {
			object = std::make_shared<GridObject>();
		}

		InitObjectByTag(tag, object);

		object->SetLayer(layer);
		if (layer == ObjectLayer::Water) {
			mEnvironments.pop_back();
			mWater = object;
		}

		object->SetModel(model);

		Vec4x4 transform;
		FileIO::ReadVal(file, transform);
		object->SetWorldTransform(transform);

		--sameObjectCount;
	}
}
void Scene::InitObjectByTag(ObjectTag tag, sptr<GridObject> object)
{
	object->SetTag(tag);

	switch (tag) {
	case ObjectTag::Unspecified:
	{
		mDynamicObjects.push_back(object);
		return;
	}

	break;
	case ObjectTag::ExplosiveSmall:
	{
		mDynamicObjects.push_back(object);

		const auto& script = object->AddComponent<Script_ExplosiveObject>();
		script->SetFX([&](const Vec3& pos) { CreateSmallExpFX(pos); });
		return;
	}

	break;
	case ObjectTag::Tank:
	case ObjectTag::Helicopter:
	case ObjectTag::ExplosiveBig:
	{
		mDynamicObjects.push_back(object);

		const auto& script = object->AddComponent<Script_ExplosiveObject>();
		script->SetFX([&](const Vec3& pos) { CreateBigExpFX(pos); });
		return;
	}

	break;
	case ObjectTag::Environment:
	{
		mEnvironments.push_back(object);
		return;
	}

	break;
	case ObjectTag::Billboard:
	{
		object->AddComponent<Script_Billboard>();
	}

	break;
	case ObjectTag::Sprite:
	{
		object->AddComponent<Script_Sprite>();
		return;
	}

	break;
	default:
		break;
	}

	mStaticObjects.emplace_back(object);
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

void Scene::RenderShadow()
{
#pragma region PrepareRender
	cmdList->SetGraphicsRootConstantBufferView(dxgi->GetGraphicsRootParamIndex(RootParam::Pass), frmResMgr->GetPassCBGpuAddr(1));
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#pragma endregion

#pragma region Shadow_Global
	RenderGridObjects(true);
	RenderTestCubes(true);
#pragma endregion
#pragma region Shadow_SkinMesh
	RenderSkinMeshObjects(true);
#pragma endregion
}

void Scene::RenderDeferred()
{
#pragma region PrepareRender
	cmdList->SetGraphicsRootConstantBufferView(dxgi->GetGraphicsRootParamIndex(RootParam::Pass), frmResMgr->GetPassCBGpuAddr(0));
	mRenderedObjects.clear();
	mSkinMeshObjects.clear();
	mTransparentObjects.clear();
	mBillboardObjects.clear();
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#pragma endregion

#pragma region Global
	RenderGridObjects();
	RenderTestCubes();
	RenderEnvironments();
#pragma endregion
#pragma region ObjectInst
	RenderInstanceObjects();
#pragma endregion
#pragma region ColorInst
	RenderBullets();
#pragma endregion
#pragma region Shadow_SkinMesh
	RenderSkinMeshObjects();
#pragma endregion
#pragma region Terrain
	RenderTerrain();
#pragma endregion
}

void Scene::RenderLights()
{
	if (mLight) {
		mLight->Render();
	}
}

void Scene::RenderFinal()
{
	// 조명에서 출력한 diffuse와 specular를 결합하여 최종 색상을 렌더링한다.
	res->Get<Shader>("Final")->Set();
	res->Get<ModelObjectMesh>("Rect")->Render();
}

void Scene::RenderForward()
{
	RenderFXObjects();
	RenderBillboards();

	RenderTransparentObjects(mTransparentObjects); 
	RenderSkyBox();
	RenderUI();

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	RenderParticles();

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Scene::RenderPostProcessing(int offScreenIndex)
{
	// 포스트 프로세싱에 필요한 상수 버퍼 뷰 설정
	PostPassConstants passConstants;
	passConstants.RT0_OffScreenIndex = offScreenIndex;
	frmResMgr->CopyData(passConstants);
	cmdList->SetGraphicsRootConstantBufferView(dxgi->GetGraphicsRootParamIndex(RootParam::PostPass), frmResMgr->GetPostPassCBGpuAddr());

	res->Get<Shader>("OffScreen")->Set();
	res->Get<ModelObjectMesh>("Rect")->Render();
}

void Scene::RenderUI()
{
	canvas->Render();
	RenderBounds(mRenderedObjects);
}

void Scene::RenderTerrain()
{
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	res->Get<Shader>("Terrain")->Set();

	if (mTerrain) {
		mTerrain->Render();
	}
}

void Scene::RenderTransparentObjects(const std::set<GridObject*>& transparentObjects)
{
	res->Get<Shader>("Transparent")->Set();
	for (auto& object : transparentObjects) {
		object->Render();
	}
	if (mWater) {
		res->Get<Shader>("Water")->Set();
		mWater->Render();
	}
}

void Scene::RenderSkyBox()
{
	mSkyBox->Render();
}

void Scene::RenderParticles()
{
	pr->Render();
}

void Scene::RenderGridObjects(bool isShadowed)
{
	if (isShadowed)
		res->Get<Shader>("Shadow_Global")->Set();
	else 
		res->Get<Shader>("Global")->Set();

	for (const auto& grid : mGrids) {
		if (grid.Empty()) {
			continue;
		}

		// TODO : 현재 인스턴싱 쪽에서 깜빡거리는 버그 발견 
		if (mainCamera->IsInFrustum(grid.GetBB())) {
			auto& objects = grid.GetObjects();
			mRenderedObjects.insert(objects.begin(), objects.end());
		}
	}

	for (auto& object : mRenderedObjects) {
		if (object->IsTransparent()) {
			mTransparentObjects.insert(object);
			continue;
		}

		switch (object->GetTag())
		{
		case ObjectTag::Billboard:
		case ObjectTag::Sprite:
			mBillboardObjects.insert(object);
			break;
		default:
			if (object->IsSkinMesh()) {
				mSkinMeshObjects.insert(object);
				break;
			}
			object->Render();
			break;
		}
	}
}

void Scene::RenderTestCubes(bool isShadowed)
{
	for (const auto& testCube : mTestCubes) {
		testCube->Render();
	}
}

void Scene::RenderSkinMeshObjects(bool isShadowed)
{
	if (isShadowed)
		res->Get<Shader>("Shadow_SkinMesh")->Set();
	else
		res->Get<Shader>("SkinMesh")->Set();

	for (auto& object : mSkinMeshObjects) {
		object->Render();
	}
}

void Scene::RenderInstanceObjects()
{
	res->Get<Shader>("ObjectInst")->Set();
	for (auto& buffer : mObjectPools) {
		buffer->Render();
	}
}

void Scene::RenderFXObjects()
{

}

void Scene::RenderEnvironments()
{
	for (auto& env : mEnvironments) {
		env->Render();
	}
}

void Scene::RenderBullets()
{
	res->Get<Shader>("ColorInst")->Set();
	for (auto& player : mPlayers) {
		if (player->IsActive()) {
			player->GetComponent<Script_ShootingPlayer>()->RenderBullets();
		}
	}
}

bool Scene::RenderBounds(const std::set<GridObject*>& renderedObjects)
{
	if (!mIsRenderBounds) {
		return false;
	}

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	res->Get<Shader>("Wire")->Set();
	RenderObjectBounds(renderedObjects);
	RenderGridBounds();
	MeshRenderer::RenderBox(Vec3(300, 107, 300), Vec3(.2f,.2f,.2f));

	return true;
}

void Scene::RenderObjectBounds(const std::set<GridObject*>& renderedObjects)
{
	for (auto& player : mPlayers) {
		if (player->IsActive()) {
			player->RenderBounds();
		}
	}

	for (auto& object : renderedObjects) {
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
		MeshRenderer::RenderPlane(pos, (float)mGridhWidth, (float)mGridhWidth);
#endif
	}
}

void Scene::RenderBillboards()
{
	res->Get<Shader>("Billboard")->Set();
	for (auto& object : mBillboardObjects) {
		object->Render();
	}
	res->Get<Shader>("Sprite")->Set();
	for (auto& object : mSpriteEffectObjects) {
		object->Render();
	}
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Update
void Scene::Start()
{
	/* Awake */
	mainCameraObject->Awake();
	mTerrain->Awake();
	ProcessObjects([](sptr<GameObject> object) {
		object->Awake();
		});

	for (auto& p : mParticles)
		p->Awake();

	/* Enable & Start */
	mTerrain->OnEnable();
	ProcessObjects([](sptr<GameObject> object) {
		object->OnEnable();
		});
	mainCameraObject->OnEnable();

	UpdateGridInfo();
}

void Scene::Update()
{
	CheckCollisions();

	UpdateObjects();
	for (auto& p : mParticles)
		p->Update();

	mainCameraObject->Update();
	mLight->Update();
	canvas->Update();
	pr->Update();

	UpdateShaderVars();
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
	}
}

void Scene::UpdateObjects()
{
	ProcessObjects([this](sptr<GridObject> object) {
		object->Update();
		});

	ProcessObjects([this](sptr<GridObject> object) {
		object->Animate();
		});

	ProcessObjects([this](sptr<GridObject> object) {
		object->LateUpdate();
		});

	UpdateFXObjects();
	UpdateSprites();
}

void Scene::UpdateFXObjects()
{

}

void Scene::UpdateSprites()
{
	for (auto it = mSpriteEffectObjects.begin(); it != mSpriteEffectObjects.end(); ) {
		auto& object = *it;
		if (object->GetComponent<Script_Sprite>()->IsEndAnimation()) {
			object->OnDestroy();
			it = mSpriteEffectObjects.erase(it);
		}
		else {
			object->Update();
			++it;
		}
	}
}
#pragma endregion





//////////////////* Others *//////////////////
void Scene::CreateSpriteEffect(Vec3 pos, float speed, float scale)
{
	sptr<GameObject> effect = std::make_shared<GameObject>();
	effect->SetModel(res->Get<MasterModel>("sprite_explosion"));
	
	effect->RemoveComponent<ObjectCollider>();
	const auto& script = effect->AddComponent<Script_Sprite>();
	script->SetSpeed(speed);
	script->SetScale(scale);

	pos.y += 2;	// 보정
	effect->SetPosition(pos);

	effect->OnEnable();
	mSpriteEffectObjects.emplace_back(effect);
}

void Scene::CreateSmallExpFX(const Vec3& pos)
{
	CreateSpriteEffect(pos, 0.0001f);
}

void Scene::CreateBigExpFX(const Vec3& pos)
{
	CreateSpriteEffect(pos, 0.025f, 5.f);
}

int Scene::GetGridIndexFromPos(Vec3 pos) const
{
	pos.x -= mGridStartPoint;
	pos.z -= mGridStartPoint;

	const int gridX = static_cast<int>(pos.x / mGridhWidth);
	const int gridZ = static_cast<int>(pos.z / mGridhWidth);

	return gridZ * mGridCols + gridX;
}

void Scene::DeleteExplodedObjects()
{
	for (auto it = mDynamicObjects.begin(); it != mDynamicObjects.end(); ) {
		auto& object = *it;
		const auto& script = object->GetComponent<Script_ExplosiveObject>();
		if (script && script->IsExploded()) {
			// remove objects in grid
			for (int index : object->GetGridIndices()) {
				mGrids[index].RemoveObject(object.get());
			}

			object->OnDestroy();
			it = mDynamicObjects.erase(it);
		}
		else {
			++it;
		}
	}
}

void Scene::ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	mPlayerScript->ProcessMouseMsg(messageID, wParam, lParam);
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
		case '0':
			for (auto& p : mParticles[1]->GetComponents<ParticleSystem>())
				p->PlayToggle();
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

	mPlayerScript->ProcessKeyboardMsg(messageID, wParam, lParam);
}

void Scene::ToggleDrawBoundings()
{
	mIsRenderBounds = !mIsRenderBounds;

	ProcessObjects([](sptr<GridObject> object) {
		object->ToggleDrawBoundings();
		});
}

void Scene::CreateFX(FXType type, const Vec3& pos)
{
	switch (type) {
	case FXType::SmallExplosion:
		CreateSmallExpFX(pos);
		break;
	case FXType::BigExplosion:
		CreateBigExpFX(pos);
		break;
	default:
		assert(0);
		break;
	}
}

void Scene::BlowAllExplosiveObjects()
{
	for (auto& object : mDynamicObjects)
	{
		const auto& script = object->GetComponent<Script_ExplosiveObject>();
		if (!script) {
			return;
		}

		script->Explode();
		for (int index : object->GetGridIndices()) {
			mGrids[index].RemoveObject(object.get());
		}
		object->OnDestroy();
	}
	mDynamicObjects.clear();
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

void Scene::UpdateObjectGrid(GridObject* object, bool isCheckAdj)
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


	// ObjectCollider가 활성화된 경우
	// 1칸 이내의 "인접 그리드(8개)와 충돌검사"
	const auto& collider = object->GetCollider();
	if(collider) {
		std::unordered_set<int> gridIndices{ gridIndex };
		const auto& objectBS = collider->GetBS();

		// BoundingSphere가 Grid 내부에 완전히 포함되면 "인접 그리드 충돌검사" X
		if (isCheckAdj && mGrids[gridIndex].GetBB().Contains(objectBS) != ContainmentType::CONTAINS) {
			const int gridX = gridIndex % mGridCols;
			const int gridZ = gridIndex / mGridCols;

			for (int offsetZ = -1; offsetZ <= 1; ++offsetZ) {
				for (int offsetX = -1; offsetX <= 1; ++offsetX) {
					const int neighborX = gridX + offsetX;
					const int neighborZ = gridZ + offsetZ;

					// 인덱스가 전체 그리드 범위 내에 있는지 확인
					if (neighborX >= 0 && neighborX < mGridCols && neighborZ >= 0 && neighborZ < mGridCols) {
						const int neighborIndex = neighborZ * mGridCols + neighborX;

						if (neighborIndex == gridIndex) {
							continue;
						}

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

			object->SetGridIndices(gridIndices);
		}
	}

	object->SetGridIndex(gridIndex);
	mGrids[gridIndex].AddObject(object);
}

void Scene::RemoveObjectFromGrid(GridObject* object)
{
	for (const int gridIndex : object->GetGridIndices()) {
		if (!IsGridOutOfRange(gridIndex)) {
			mGrids[gridIndex].RemoveObject(object);
		}
	}

	object->ClearGridIndices();
}

sptr<GridObject> Scene::Instantiate(const std::string& modelName, bool enable) const
{
	const auto& model = res->Get<MasterModel>(modelName);
	if (!model) {
		return nullptr;
	}

	sptr<GridObject> instance = std::make_shared<GridObject>();
	instance->SetModel(model);
	if (enable) {
		instance->OnEnable();
	}

	return instance;
}



void Scene::ProcessObjects(std::function<void(sptr<GridObject>)> processFunc)
{
	for (auto& player : mPlayers) {
		processFunc(player);
	}

	for (auto& object : mStaticObjects) {
		processFunc(object);
	}

	for (auto& object : mDynamicObjects) {
		processFunc(object);
	}
}