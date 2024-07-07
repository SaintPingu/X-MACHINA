#pragma region Include
#include "EnginePch.h"
#include "Scene.h"
#include "DXGIMgr.h"
#include "MultipleRenderTarget.h"
#include "FrameResource.h"
#include "Ssao.h"

#include "ResourceMgr.h"
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
#include "Component/UI.h"
#include "Component/Camera.h"
#include "Component/Collider.h"
#include "Component/Component.h"
#include "Component/ParticleSystem.h"
#include "AbilityMgr.h"
#pragma endregion

#include "TestCube.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region C/Dtor
namespace {
	constexpr int kGridXCount     = 20;
	constexpr int kGridZCount     = 10;
	constexpr Vec3 kBorderPos     = Vec3(500, 20, 250);		// center of border
	constexpr Vec3 kBorderExtents = Vec3(1000, 500, 500);		// extents of border
}

Scene::Scene()
	:
	mMapBorder(kBorderPos, kBorderExtents),
	mGridXLength(static_cast<int>(mMapBorder.Extents.x / kGridXCount)),
	mGridZLength(static_cast<int>(mMapBorder.Extents.z / kGridZCount)),
	mLight(std::make_shared<Light>())
{

}

void Scene::Release()
{
	FRAME_RESOURCE_MGR->WaitForGpuComplete();

	ReleaseObjects();

	ProcessAllObjects([](sptr<Object> object) {
		object->Destroy();
		});

	mGameManager->OnDestroy();
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Getter
float Scene::GetTerrainHeight(float x, float z) const
{
	assert(mTerrain);

	return mTerrain->GetHeight(x, z);
}

std::vector<sptr<GameObject>> Scene::GetAllObjects() const
{
	std::vector<sptr<GameObject>> result;
	result.reserve(mEnvironments.size() + mStaticObjects.size() + mDynamicObjects.size());
	result.insert(result.end(), mEnvironments.begin(), mEnvironments.end());
	result.insert(result.end(), mStaticObjects.begin(), mStaticObjects.end());
	result.insert(result.end(), mDynamicObjects.begin(), mDynamicObjects.end());

	return result;
}

#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region DirectX
void Scene::ReleaseUploadBuffers()
{
	MeshRenderer::ReleaseUploadBuffers();
}

void Scene::UpdateShaderVars()
{
	UpdateMainPassCB();
#ifndef RENDER_FOR_SERVER
	UpdateShadowPassCB();
	UpdateSsaoCB();
#endif
	UpdateMaterialBuffer();
}

void Scene::UpdateMainPassCB()
{
	Matrix proj = MAIN_CAMERA->GetProjMtx();
	PassConstants passCB;
	passCB.MtxView = MAIN_CAMERA->GetViewMtx().Transpose();
	passCB.MtxProj = MAIN_CAMERA->GetProjMtx().Transpose();
	passCB.MtxInvProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	passCB.MtxShadow = mLight->GetShadowMtx().Transpose();
	passCB.MtxNoLagView = MAIN_CAMERA->GetNoLagViewtx().Transpose();
	passCB.CameraPos = MAIN_CAMERA->GetPosition();
	passCB.CameraRight = MAIN_CAMERA->GetRight();
	passCB.CameraUp = MAIN_CAMERA->GetUp();
	passCB.DeltaTime = DeltaTime();
	passCB.TotalTime = Timer::I->GetTotalTime();
	passCB.FrameBufferWidth = DXGIMgr::I->GetWindowWidth();
	passCB.FrameBufferHeight = DXGIMgr::I->GetWindowHeight();
	passCB.SkyBoxIndex = mSkyBox->GetTexture()->GetSrvIdx();
	passCB.DefaultDsIndex = RESOURCE<Texture>("DefaultDepthStencil")->GetSrvIdx();
	passCB.ShadowDsIndex = RESOURCE<Texture>("ShadowDepthStencil")->GetSrvIdx();
	passCB.CustomDsIndex = RESOURCE<Texture>("CustomDepthStencil")->GetSrvIdx();
	passCB.RT0G_PositionIndex = RESOURCE<Texture>("PositionTarget")->GetSrvIdx();
	passCB.RT1G_NormalIndex = RESOURCE<Texture>("NormalTarget")->GetSrvIdx();
	passCB.RT2G_DiffuseIndex = RESOURCE<Texture>("DiffuseTarget")->GetSrvIdx();
	passCB.RT3G_EmissiveIndex = RESOURCE<Texture>("EmissiveTarget")->GetSrvIdx();
	passCB.RT4G_MetallicSmoothnessIndex = RESOURCE<Texture>("MetallicSmoothnessTarget")->GetSrvIdx();
	passCB.RT5G_OcclusionIndex = RESOURCE<Texture>("OcclusionTarget")->GetSrvIdx();
	passCB.RT0L_DiffuseIndex = RESOURCE<Texture>("DiffuseAlbedoTarget")->GetSrvIdx();
	passCB.RT1L_SpecularIndex = RESOURCE<Texture>("SpecularAlbedoTarget")->GetSrvIdx();
	passCB.RT2L_AmbientIndex = RESOURCE<Texture>("AmbientTarget")->GetSrvIdx();
	passCB.RT0S_SsaoIndex = RESOURCE<Texture>("SSAOTarget_0")->GetSrvIdx();
	passCB.RT0O_OffScreenIndex = RESOURCE<Texture>("OffScreenTarget")->GetSrvIdx();
	passCB.BloomIndex = RESOURCE<Texture>("BloomTarget")->GetSrvIdx();
#ifndef RENDER_FOR_SERVER
	passCB.LiveObjectDissolveIndex = RESOURCE<Texture>("LiveObjectDissolve")->GetSrvIdx();
	passCB.BuildingDissolveIndex = RESOURCE<Texture>("Dissolve_01_05")->GetSrvIdx();
#endif
	passCB.LightCount = mLight->GetLightCount();
	passCB.GlobalAmbient = Vec4(0.4f, 0.4f, 0.4f, 1.f);
	passCB.FilterOption = DXGIMgr::I->GetFilterOption();
	passCB.ShadowIntensity = 0.0f;
	passCB.FogColor = Colors::Gray;
	memcpy(&passCB.Lights, mLight->GetSceneLights().get()->Lights.data(), sizeof(passCB.Lights));

	FRAME_RESOURCE_MGR->CopyData(0, passCB);
}

void Scene::UpdateShadowPassCB()
{
	PassConstants passCB;
	passCB.MtxView = mLight->GetLightViewMtx().Transpose();
	passCB.MtxProj = mLight->GetLightProjMtx().Transpose();
	passCB.LiveObjectDissolveIndex = RESOURCE<Texture>("LiveObjectDissolve")->GetSrvIdx();

	FRAME_RESOURCE_MGR->CopyData(1, passCB);
}

void Scene::UpdateSsaoCB()
{
	SsaoConstants ssaoCB;

	Matrix mtxProj = MAIN_CAMERA->GetProjMtx();
	Matrix mtxTex = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f };

	ssaoCB.MtxInvProj = MAIN_CAMERA->GetProjMtx().Invert().Transpose();
	ssaoCB.MtxProjTex = (mtxProj * mtxTex).Transpose();
	DXGIMgr::I->GetSsao()->GetOffsetVectors(ssaoCB.OffsetVectors);

	// for Blur 
	auto blurWeights = Filter::CalcGaussWeights(2.5f);
	ssaoCB.BlurWeights[0] = Vec4(&blurWeights[0]);
	ssaoCB.BlurWeights[1] = Vec4(&blurWeights[4]);
	ssaoCB.BlurWeights[2] = Vec4(&blurWeights[8]);

	auto ssaoTarget = RESOURCE<Texture>("SSAOTarget_0");
	ssaoCB.InvRenderTargetSize = Vec2{ 1.f / ssaoTarget->GetWidth(), 1.f / ssaoTarget->GetHeight() };

	// coordinates given in view space.
	ssaoCB.OcclusionRadius = 0.5f;
	ssaoCB.OcclusionFadeStart = 0.2f;
	ssaoCB.OcclusionFadeEnd = 1.0f;
	ssaoCB.SurfaceEpsilon = 0.05f;
	ssaoCB.AccessContrast = 12;

	ssaoCB.RandomVectorIndex = RESOURCE<Texture>("RandomVector")->GetSrvIdx();

	FRAME_RESOURCE_MGR->CopyData(ssaoCB);
}

void Scene::UpdateAbilityCB(int& idx, const AbilityConstants& value)
{
	FRAME_RESOURCE_MGR->CopyData(idx, value);
}

void Scene::SetAbilityCB(int idx) const
{
	CMD_LIST->SetGraphicsRootConstantBufferView(DXGIMgr::I->GetGraphicsRootParamIndex(RootParam::Ability), FRAME_RESOURCE_MGR->GetAbilityCBGpuAddr(idx));
}

void Scene::UpdateMaterialBuffer()
{
	ResourceMgr::I->ProcessFunc<MasterModel>(
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
	Canvas::I->Init();

	// load models
	LoadSceneObjects("Import/Scene.bin");
	mGameManager   = std::make_shared<Object>();

	// build settings
	BuildTerrain();

	// build static meshes
	MeshRenderer::BuildMeshes();

	// skybox
	mSkyBox = std::make_shared<SkyBox>();
}

void Scene::ReleaseObjects()
{
	MeshRenderer::Release();
}

void Scene::BuildTerrain()
{
	mTerrain = std::make_shared<Terrain>("Import/Terrain.bin");

	BuildGrid();
}

void Scene::BuildGrid()
{
	constexpr float kMaxHeight = 300.f;	// for 3D grid

	// recalculate scene grid size
	const int adjustedX = Math::GetNearestMultiple((int)mMapBorder.Extents.x, mGridXLength);
	const int adjustedZ = Math::GetNearestMultiple((int)mMapBorder.Extents.z, mGridZLength);
	mMapBorder.Extents = Vec3((float)adjustedX, mMapBorder.Extents.y, (float)adjustedZ);

	// set grid start pos
	mGridStartPoint = mMapBorder.Center.x - mMapBorder.Extents.x / 2;

	// set grid count
	mGridXCount = adjustedX / mGridXLength;
	mGridZCount = adjustedZ / mGridZLength;
	const int gridCount = mGridXCount * mGridZCount;
	mGrids.resize(gridCount);

	// set grid bounds
	const float gridExtentX = (float)mGridXLength / 2.0f;
	const float gridExtentZ = (float)mGridZLength / 2.0f;
	for (int z = 0; z < mGridZCount; ++z) {
		for (int x = 0; x < mGridXCount; ++x) {
			float gridX = (mGridXLength * x) + ((float)mGridXLength / 2) + mGridStartPoint;
			float gridZ = (mGridZLength * z) + ((float)mGridZLength / 2) + mGridStartPoint;

			BoundingBox bb{};
			bb.Center = Vec3(gridX, kMaxHeight, gridZ);
			bb.Extents = Vec3(gridExtentX, kMaxHeight, gridExtentZ);

			int index = (z * mGridXCount) + x;
			mGrids[index] = std::make_shared<Grid>(index, mGridXLength, mGridZLength, bb);
		}
	}
}

void Scene::UpdateGridInfo()
{
	ProcessActiveObjects([this](sptr<GridObject> object) {
		UpdateObjectGrid(object.get());
		});

	mTerrain->UpdateGrid();
}


void Scene::LoadSceneObjects(const std::string& fileName)
{
	std::ifstream file = FileIO::OpenBinFile(fileName);

	mLight->BuildLights(file);
	LoadGameObjects(file);
}

void Scene::LoadGameObjects(std::ifstream& file)
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
			model = RESOURCE<MasterModel>(meshName);

			FileIO::ReadString(file, token); //"<Transforms>:"
			FileIO::ReadVal(file, sameObjectCount);

			FileIO::ReadString(file, token); //"<IsInstancing>:"
			FileIO::ReadVal(file, isInstancing);
			
			if (isInstancing) {
				objectPool = CreateObjectPool(model, sameObjectCount, [&](rsptr<InstObject> object) {
					object->SetTag(tag);
					});
			}

			if (tag == ObjectTag::Unspecified) {
				std::cout << "[WARNING] Untagged Object : " << model->GetName() << std::endl;
			}
		}

		if (isInstancing) {
			// �ν��Ͻ� ��ü�� ������ ��ü�� �޾ƿ´�.
			object = objectPool->Get(false);
		}
		else {
			object = std::make_shared<GridObject>();
			object->SetModel(model);
			InitObjectByTag(tag, object);
		}


		object->SetLayer(layer);

		Matrix transform;
		FileIO::ReadVal(file, transform);
		object->SetWorldTransform(transform);

		--sameObjectCount;
	}
}

void Scene::InitObjectByTag(ObjectTag tag, sptr<GridObject> object)
{
	object->SetTag(tag);
	ObjectType type = object->GetType();

	switch (type) {
	case ObjectType::Dynamic:
		mDynamicObjects.push_back(object);
		break;
	case ObjectType::Env:
		mEnvironments.push_back(object);
		break;
	default:
		mStaticObjects.push_back(object);
		break;
	}
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


void Scene::ClearRenderedObjects()
{
	mRenderedObjects.clear();
	mSkinMeshObjects.clear();
	mTransparentObjects.clear();
	mGridObjects.clear();
	mObjectsByShader.clear();
}


void Scene::RenderShadow()
{
	if (!DXGIMgr::I->GetFilterOption(FilterOption::Shadow))
		return;

#pragma region PrepareRender
	CMD_LIST->SetGraphicsRootConstantBufferView(DXGIMgr::I->GetGraphicsRootParamIndex(RootParam::Pass), FRAME_RESOURCE_MGR->GetPassCBGpuAddr(1));
#pragma endregion
	
#pragma region Shadow_Global
	RenderGridObjects(RenderType::Shadow);
#pragma endregion
#pragma region Shadow_SkinMesh
	RenderSkinMeshObjects(RenderType::Shadow);
#pragma endregion
#pragma region Shadow_ObjectInst
	RenderInstanceObjects(RenderType::Shadow);
#pragma endregion
}


void Scene::ApplyDynamicContext()
{
	DynamicEnvironmentMappingManager::I->Render(mSkinMeshObjects);
}

void Scene::RenderDynamicEnvironmentMappingObjects()
{
	RenderGridObjects(RenderType::DynamicEnvironmentMapping);
	RenderInstanceObjects(RenderType::DynamicEnvironmentMapping);
	RenderTerrain(RenderType::DynamicEnvironmentMapping);
	RenderSkyBox(RenderType::DynamicEnvironmentMapping);
}


void Scene::RenderDeferred()
{
#pragma region PrepareRender
	CMD_LIST->SetGraphicsRootConstantBufferView(DXGIMgr::I->GetGraphicsRootParamIndex(RootParam::Pass), FRAME_RESOURCE_MGR->GetPassCBGpuAddr(0));
#pragma endregion
#pragma region Globald
	RenderGridObjects(RenderType::Deferred);
	RenderEnvironments();
#pragma endregion
#pragma region ObjectInst
	RenderInstanceObjects(RenderType::Deferred);
#pragma endregion
#pragma region SkinMesh
	RenderSkinMeshObjects(RenderType::Deferred);
#pragma endregion
#pragma region Terrain
	RenderTerrain(RenderType::Deferred);
#pragma endregion
}


void Scene::RenderCustomDepth()
{
	if (!DXGIMgr::I->GetFilterOption(FilterOption::Custom))
		return;

#pragma region CustomDepth_SkinMesh
	RenderSkinMeshObjects(RenderType::CustomDepth);
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
	// �������� ����� diffuse�� specular�� �����Ͽ� ���� ������ �������Ѵ�.
	RESOURCE<Shader>("Final")->Set();
	RESOURCE<ModelObjectMesh>("Rect")->Render();
}

void Scene::RenderForward()
{
	RenderTransparentObjects(); 
	RenderDissolveObjects();
	RenderAfterSkinImage();
	RenderSkyBox(RenderType::Forward);

	RenderAbilities();
	RenderParticles();
}

void Scene::RenderBloom()
{
	RESOURCE<Shader>("Bloom")->Set();
	RESOURCE<ModelObjectMesh>("Rect")->Render();
}

void Scene::RenderPostProcessing(int offScreenIndex)
{
	// ����Ʈ ���μ��̿� �ʿ��� ��� ���� �� ����
	PostPassConstants passConstants;
	passConstants.RT0_OffScreenIndex = offScreenIndex;
	FRAME_RESOURCE_MGR->CopyData(passConstants);
	CMD_LIST->SetGraphicsRootConstantBufferView(DXGIMgr::I->GetGraphicsRootParamIndex(RootParam::PostPass), FRAME_RESOURCE_MGR->GetPostPassCBGpuAddr());

	RESOURCE<Shader>("OffScreen")->Set();
	RESOURCE<ModelObjectMesh>("Rect")->Render();
}

void Scene::RenderUI()
{
	Canvas::I->Render();
	RenderBounds();
}

void Scene::RenderDeferredForServer()
{
	CMD_LIST->SetGraphicsRootConstantBufferView(DXGIMgr::I->GetGraphicsRootParamIndex(RootParam::Pass), FRAME_RESOURCE_MGR->GetPassCBGpuAddr(0));

	Scene::I->UpdateRenderedObjects();
}

void Scene::RenderTerrain(RenderType type)
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

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

void Scene::RenderAfterSkinImage()
{
	RESOURCE<Shader>("AfterSkinImage")->Set();

	for (auto& object : mObjectsByShader[ObjectTag::AfterSkinImage]) {
		object->Render();
	}
}

void Scene::RenderTransparentObjects()
{
	RESOURCE<Shader>("Transparent")->Set();
	for (auto& object : mTransparentObjects) {
		object->Render();
	}
}

void Scene::RenderDissolveObjects()
{
	RESOURCE<Shader>("Dissolve")->Set();
	// [destroyTime]�� ��� �� ��ü ����
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

void Scene::RenderSkyBox(RenderType type)
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

void Scene::RenderParticles()
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	ParticleManager::I->Render();
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Scene::RenderAbilities()
{
	AbilityMgr::I->Render();
}

void Scene::RenderGridObjects(RenderType type)
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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

void Scene::RenderSkinMeshObjects(RenderType type)
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

void Scene::RenderInstanceObjects(RenderType type)
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

void Scene::RenderObjectsWithFrustumCulling(std::set<GridObject*>& objects, RenderType type)
{
	if (type == RenderType::Shadow) {
		for (auto it = objects.begin(); it != objects.end();) {
			if ((*it)->GetUseShadow()) {
				(*it)->Render();
			}

			if (!MAIN_CAMERA->GetFrustum().Intersects((*it)->GetCollider()->GetBS())) {
				it = objects.erase(it);
			}
			else {
				++it;
			}
		}
	}
	else {
		for (const auto& object : objects) {
			object->Render();
		}
	}
}

void Scene::RenderEnvironments()
{
	for (auto& env : mEnvironments) {
		env->Render();
	}
}

bool Scene::RenderBounds()
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	RESOURCE<Shader>("Wire")->Set();
	MeshRenderer::RenderBox(Vec3(100, 13.5f, 105), Vec3(.2f,.2f,.2f));

	//// ���� ����Ʈ�� �ʷϻ����� ���
	//for (auto& path : mOpenList) {
	//	path.y = GetTerrainHeight(path.x, path.z);
	//	MeshRenderer::RenderBox(path, Vec3{ 0.1f, 0.1f, 0.1f }, Vec4{ 0.f, 1.f, 0.f, 1.f });
	//}

	//// Ŭ����� ����Ʈ�� ���������� ���
	//for (auto& path : mClosedList) {
	//	path.y = GetTerrainHeight(path.x, path.z);
	//	MeshRenderer::RenderBox(path, Vec3{ 0.1f, 0.1f, 0.1f }, Vec4{ 1.f, 0.f, 0.f, 1.f });
	//}

	if (!mIsRenderBounds) {
		return false;
	}

	RenderObjectBounds();
	RenderGridBounds();

	return true;
}

void Scene::RenderObjectBounds()
{
	for (auto& object : mRenderedObjects) {
		object->RenderBounds();
	}
}

//#define DRAW_SCENE_GRID_3D
void Scene::RenderGridBounds()
{
	for (const auto& grid : mSurroundGrids) {
#ifdef DRAW_SCENE_GRID_3D
		MeshRenderer::Render(grid->GetBB());
#else
		constexpr float kGirdHeight = 0.5f;
		Vec3 pos = grid->GetBB().Center;
		pos.y = kGirdHeight;
		MeshRenderer::RenderPlane(pos, (float)mGridXLength, (float)mGridZLength);
#endif
	}
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Update
void Scene::Start()
{
	/* Awake */
	mTerrain->Awake();
	MainCamera::I->Awake();
	ProcessAllObjects([](sptr<Object> object) {
		object->Awake();
		});

	mGameManager->Awake();

	/* Enable */
	mTerrain->SetActive(true);
	MainCamera::I->SetActive(true);
	ProcessAllObjects([](sptr<Object> object) {
		object->SetActive(true);
		});
	mGameManager->SetActive(true);

	UpdateGridInfo();
}

void Scene::Update()
{
	UpdateRenderedObjects();
	
	ProcessCollisions();
	mGameManager->Update();
	UpdateObjects();
	mGameManager->LateUpdate();
	ParticleManager::I->Update();

	MainCamera::I->Update();
	MAIN_CAMERA->UpdateViewMtx();
	mLight->Update();
	Canvas::I->Update();
	UpdateSurroundGrids();

	UpdateShaderVars();

	PopObjectBuffer();

}

void Scene::ProcessCollisions()
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

void Scene::CheckCollisionCollider(rsptr<Collider> collider, std::vector<GridObject*>& out, CollisionType type) const
{
	int gridIndex = GetGridIndexFromPos(collider->GetCenter());
	for (const auto& grid : GetNeighborGrids(gridIndex, true)) {
		if (!collider->Intersects(grid->GetBB())) {
			continue;
		}

		grid->CheckCollisions(collider, out, type);
	}
}

float Scene::CheckCollisionsRay(int gridIndex, const Ray& ray) const
{
	// �����¿�, �밢�� �׸��嵵 üũ �ʿ�
	return mGrids[gridIndex]->CheckCollisionsRay(ray);
}

void Scene::UpdateObjects()
{
	ProcessActiveObjects([this](sptr<Object> object) {
		if (object->IsActive()) {
			object->Update();
		}
		});

	// TODO : �ɷ��� ������ ������Ʈ�� �ִϸ��̼��ϵ��� �ߴµ� 
	// ���Ŀ� �����̰� �ν��Ͻ� ��ü�� ���Խ��Ѿ� ��
	AnimateObjects();

	ProcessActiveObjects([this](sptr<Object> object) {
		if (object->IsActive()) {
			object->LateUpdate();
		}
		});
}

void Scene::AnimateObjects()
{
	for (auto& object : mRenderedObjects) {
		if (object->IsActive()) {
			object->Animate();
		}
	}
}

void Scene::UpdateRenderedObjects()
{
	if (mRenderedObjects.empty()) {
		for (const auto& grid : mGrids) {
			if (grid->Empty()) {
				continue;
			}

			if (MAIN_CAMERA->GetFrustumShadow().Intersects(grid->GetBB())) {
				auto& objects = grid->GetObjects();
				for (auto& object : objects) {
					if (MAIN_CAMERA->GetFrustumShadow().Intersects(object->GetCollider()->GetBS())) {
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

			if (!object->IsActive()) {
				disabledObjects.insert(object);
				continue;
			}
			if (object->IsTransparent()) {
				mTransparentObjects.insert(object);
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





void Scene::PopObjectBuffer()
{
	if (!mDynamicObjectBuffer.empty()) {
		mDynamicObjects.insert(mDynamicObjects.end(), mDynamicObjectBuffer.begin(), mDynamicObjectBuffer.end());
		mDynamicObjectBuffer.clear();
	}
}

//////////////////* Others *//////////////////
rsptr<Grid> Scene::GetGridFromPos(const Vec3& pos)
{
	return mGrids[GetGridIndexFromPos(pos)];
}

int Scene::GetGridIndexFromPos(Vec3 pos) const
{
	pos.x -= mGridStartPoint;
	pos.z -= mGridStartPoint;

	const int gridX = static_cast<int>(pos.x / mGridXLength);
	const int gridZ = static_cast<int>(pos.z / mGridZLength);

	return gridZ * mGridXCount + gridX;
}

Pos Scene::GetTileUniqueIndexFromPos(const Vec3& pos) const
{
	// ���� ���������κ��� Ÿ���� ���� �ε����� ���
	const int tileGroupIndexX = static_cast<int>((pos.x - mGridStartPoint) / Grid::mkTileWidth);
	const int tileGroupIndexZ = static_cast<int>((pos.z - mGridStartPoint) / Grid::mkTileHeight);

	return Pos{ tileGroupIndexZ, tileGroupIndexX };
}

Vec3 Scene::GetTilePosFromUniqueIndex(const Pos& index) const
{
	// Ÿ���� ���� �ε����κ��� ���� �������� ���
	const float posX = index.X * Grid::mkTileWidth + mGridStartPoint;
	const float posZ = index.Z * Grid::mkTileHeight + mGridStartPoint;

	return Vec3{ posX, 0, posZ };
}

Tile Scene::GetTileFromPos(const Vec3& pos) const
{
	return GetTileFromUniqueIndex(GetTileUniqueIndexFromPos(pos));
}

Tile Scene::GetTileFromUniqueIndex(const Pos& index) const
{
	// Ÿ���� ���� �ε����κ��� Ÿ���� ���� ��ȯ
	const int gridX = static_cast<int>(index.X * Grid::mkTileWidth / mGridXLength);
	const int gridZ = static_cast<int>(index.Z * Grid::mkTileHeight / mGridZLength);

	const int tileX = index.X % Grid::mTileCols;
	const int tileZ = index.Z % Grid::mTileRows;

	return mGrids[gridZ * mGridXCount + gridX]->GetTileFromUniqueIndex(Pos{tileZ, tileX});
}

void Scene::SetTileFromUniqueIndex(const Pos& index, Tile tile)
{
	// Ÿ���� ���� �ε����κ��� Ÿ���� ���� ��ȯ
	const int gridX = static_cast<int>(index.X * Grid::mkTileWidth / mGridXLength);
	const int gridZ = static_cast<int>(index.Z * Grid::mkTileHeight / mGridZLength);

	const int tileX = index.X % Grid::mTileCols;
	const int tileZ = index.Z % Grid::mTileRows;

	mGrids[gridZ * mGridXCount + gridX]->SetTileFromUniqueIndex(Pos{ tileZ, tileX }, tile);
}


void Scene::ToggleDrawBoundings()
{
	mIsRenderBounds = !mIsRenderBounds;

	ProcessAllObjects([](sptr<GridObject> object) {
		object->ToggleDrawBoundings();
		});
}

void Scene::ToggleFilterOptions()
{
	static UINT8 filterIdx = 0;
	static std::array<DWORD, 6> values = { 0x004, 0x008, 0x010, 0x020, 0x080, 0x002 };
	DXGIMgr::I->SetFilterOptions(values[filterIdx++]);
	filterIdx %= values.size();

	if (filterIdx == 0)
		std::reverse(values.begin(), values.end());
}

void Scene::SetFilterOptions(DWORD option)
{
	DXGIMgr::I->SetFilterOptions(option);
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


	// ObjectCollider�� Ȱ��ȭ�� ���
	// 1ĭ �̳��� "���� �׸���(8��)�� �浹�˻�"
	const auto& collider = object->GetCollider();
	if (collider && collider->IsActive()) {
		std::unordered_set<int> gridIndices{ gridIndex };
		const auto& objectBS = collider->GetBS();

		// BoundingSphere�� Grid ���ο� ������ ���ԵǸ� "���� �׸��� �浹�˻�" X
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

void Scene::RemoveObjectFromGrid(GridObject* object)
{
	for (const int gridIndex : object->GetGridIndices()) {
		if (!IsGridOutOfRange(gridIndex)) {
			mGrids[gridIndex]->RemoveObject(object);
		}
	}

	object->ClearGridIndices();
}

void Scene::UpdateSurroundGrids()
{
	const Vec3 cameraPos = MAIN_CAMERA->GetPosition();
	const int currGridIndex = GetGridIndexFromPos(cameraPos);

	// �׸��� �ε����� ����� ��쿡�� �ֺ� �׸��带 ������Ʈ
	static int prevGridIndex;
	if (prevGridIndex != currGridIndex) {
		mSurroundGrids.clear();
		mSurroundGrids = GetNeighborGrids(currGridIndex, true);
		prevGridIndex = currGridIndex;
	}
}

sptr<GridObject> Scene::Instantiate(const std::string& modelName, ObjectTag tag, ObjectLayer layer, bool enable)
{
	const auto& model = RESOURCE<MasterModel>(modelName);
	if (!model) {
		return nullptr;
	}

	sptr<GridObject> instance = std::make_shared<GridObject>();
	instance->SetModel(model);
	instance->SetTag(tag);
	instance->SetLayer(layer);
	instance->SetName(modelName);
	if (enable) {
		instance->SetActive(true);
	}
	mDynamicObjectBuffer.push_back(instance);

	return instance;
}


void Scene::RemoveDynamicObject(GridObject* target)
{
	for (size_t i = 0; i < mDynamicObjects.size();++i) {
		auto& object = mDynamicObjects[i];
		if (object.get() == target) {
			if (object->IsSkinMesh()) {
				mDissolveObjects.insert(object);
			}
			mDestroyObjects.insert(i);
			object = nullptr;
			return;
		}
	}
}


sptr<ObjectPool> Scene::CreateObjectPool(const std::string& modelName, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc)
{
	return CreateObjectPool(RESOURCE<MasterModel>(modelName), maxSize, objectInitFunc);
}

sptr<ObjectPool> Scene::CreateObjectPool(rsptr<const MasterModel> model, int maxSize, const std::function<void(rsptr<InstObject>)>& objectInitFunc)
{
	sptr<ObjectPool> pool = mObjectPools.emplace_back(std::make_shared<ObjectPool>(model, maxSize));
	pool->CreateObjects<InstObject>(objectInitFunc);

	return pool;
}

std::vector<sptr<Grid>> Scene::GetNeighborGrids(int gridIndex, bool includeSelf) const
{
	std::vector<sptr<Grid>> result;
	result.reserve(9);

	const int gridX = gridIndex % mGridXCount;
	const int gridZ = gridIndex / mGridXCount;

	for (int offsetZ = -1; offsetZ <= 1; ++offsetZ) {
		for (int offsetX = -1; offsetX <= 1; ++offsetX) {
			const int neighborX = gridX + offsetX;
			const int neighborZ = gridZ + offsetZ;

			// �ε����� ��ü �׸��� ���� ���� �ִ��� Ȯ��
			if (neighborX >= 0 && neighborX < mGridXCount && neighborZ >= 0 && neighborZ < mGridZCount) {
				const int neighborIndex = (neighborZ * mGridXCount) + neighborX;

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

void Scene::ToggleFullScreen()
{
	DXGIMgr::I->ToggleFullScreen();
}

std::vector<sptr<GridObject>> Scene::FindObjectsByName(const std::string& name)
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

void Scene::ProcessActiveObjects(std::function<void(sptr<GridObject>)> processFunc)
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

void Scene::ProcessAllObjects(std::function<void(sptr<GridObject>)> processFunc)
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

void Scene::RemoveDesrtoyedObjects()
{
	for (auto& index : mDestroyObjects | std::ranges::views::reverse) {
		mDynamicObjects[index] = mDynamicObjects.back();
		mDynamicObjects.pop_back();
	}

	mDestroyObjects.clear();
}




ObjectTag Scene::GetTagByString(const std::string& tag)
{
	switch (Hash(tag)) {
	case Hash("Building"):
		return ObjectTag::Building;

	case Hash("Dissolve_Building"):
		return ObjectTag::DissolveBuilding;

	case Hash("Background"):
		return ObjectTag::Environment;

	case Hash("Enemy"):
		return ObjectTag::Enemy;

	case Hash("Prop"):
		return ObjectTag::Prop;

	case Hash("Dynamic"):
		return ObjectTag::Dynamic;

	case Hash("Crate"):
		return ObjectTag::Crate;

	default:
		//assert(0);
		break;
	}

	return ObjectTag::Unspecified;
}

ObjectLayer Scene::GetLayerByNum(int num)
{
	switch (num) {
	case 0:
		return ObjectLayer::Default;

	case 3:
		return ObjectLayer::Transparent;

	case 4:
		return ObjectLayer::Water;

	default:
		break;
	}

	return ObjectLayer::Default;
}