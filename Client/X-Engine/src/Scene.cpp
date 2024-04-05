#pragma region Include
#include "EnginePch.h"
#include "Scene.h"
#include "DXGIMgr.h"
#include "MultipleRenderTarget.h"
#include "FrameResource.h"

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

#include "TestCube.h"
#include "Ssao.h"
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

	ProcessAllObjects([](sptr<GameObject> object) {
		object->OnDestroy();
		});

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

std::vector<sptr<GameObject>> Scene::GetAllObjects() const
{
	std::vector<sptr<GameObject>> result;
	result.reserve(mEnvironments.size() + mStaticObjects.size() + mDynamicObjects.size());
	result.insert(result.end(), mEnvironments.begin(), mEnvironments.end());
	result.insert(result.end(), mStaticObjects.begin(), mStaticObjects.end());
	result.insert(result.end(), mDynamicObjects.begin(), mDynamicObjects.end());

	return result;
}

std::vector<sptr<GameObject>> Scene::GetAllPartilceSystems() const
{
	return mParticles;
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
	passCB.FrameBufferWidth				= dxgi->GetWindowWidth();
	passCB.FrameBufferHeight			= dxgi->GetWindowHeight();
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
	mGameManager = std::make_shared<Object>();

	// build settings
	BuildTerrain();
	BuildNavMesh();
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

void Scene::BuildTerrain()
{
	mTerrain = std::make_shared<Terrain>("Import/Terrain.bin");

	BuildGrid();
}

void Scene::BuildNavMesh()
{
	mNavMesh = FileIO::ModelIO::LoadNavMesh("Import/NavMesh.bin");
}

void Scene::BuildTest()
{
	mParticles.resize(1);

#pragma region MagicMissile
	mParticles[0] = std::make_shared<GameObject>();
	mParticles[0]->SetPosition(Vec3{ 97.5f, GetTerrainHeight(97.5f, 100.f) + 2.f, 104.f });
	mParticles[0]->AddComponent<ParticleSystem>()->Load("Small_MagicMissile_Out");
	mParticles[0]->AddComponent<ParticleSystem>()->Load("Big_MagicMissile_Out");
	mParticles[0]->AddComponent<ParticleSystem>()->Load("Big_MagicMissile_Light");
	mParticles[0]->AddComponent<ParticleSystem>()->Load("Small_MagicMissile_Light");
#pragma endregion

//#pragma region ShapeTest
//	mParticles[1] = std::make_shared<GameObject>();
//	mParticles[1]->SetPosition(Vec3{ 103.f, GetTerrainHeight(103.f, 100.f) + 2.f, 104.f });
//	auto& component = mParticles[1]->AddComponent<ParticleSystem>()->Load("Green");
//	component->GetPSCD().StartLifeTime = 0.3f;
//	component->GetPSCD().Emission.SetBurst(100);
//	component->GetPSCD().Duration = 0.2f;
//	component->GetPSCD().Shape.SetSphere(1.5f, 0.f, 360.f, true);
//#pragma endregion
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
			model = res->Get<MasterModel>(meshName);

			FileIO::ReadString(file, token); //"<Transforms>:"
			FileIO::ReadVal(file, sameObjectCount);

			FileIO::ReadString(file, token); //"<IsInstancing>:"
			FileIO::ReadVal(file, isInstancing);

			if (isInstancing) {
				objectPool = CreateObjectPool(model, sameObjectCount);
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

		object->SetModel(model);

		Matrix transform;
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
	case ObjectTag::Tank:
	case ObjectTag::Helicopter:
	case ObjectTag::ExplosiveBig:
	case ObjectTag::ExplosiveSmall:
	{
		mDynamicObjects.push_back(object);
		return;
	}

	break;
	case ObjectTag::Environment:
	{
		mEnvironments.push_back(object);
		return;
	}

	break;
	default:
		break;
	}

	mStaticObjects.push_back(object);
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
	RenderEnvironments();
#pragma endregion
#pragma region ObjectInst
	RenderInstanceObjects();
#pragma endregion
#pragma region ColorInst
	
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
	RenderTransparentObjects(mTransparentObjects); 
	RenderSkyBox();

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
	//canvas->Render();
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
			object->ComputeWorldTransform();
			object->Render();
			break;
		}
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

void Scene::RenderEnvironments()
{
	for (auto& env : mEnvironments) {
		env->Render();
	}
}

bool Scene::RenderBounds(const std::set<GridObject*>& renderedObjects)
{
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	res->Get<Shader>("Wire")->Set();
	MeshRenderer::RenderBox(Vec3(100, 13.5f, 105), Vec3(.2f,.2f,.2f));
	if (!mIsRenderBounds) {
		return false;
	}

	RenderObjectBounds(renderedObjects);
	RenderGridBounds();

	return true;
}

void Scene::RenderObjectBounds(const std::set<GridObject*>& renderedObjects)
{
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
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Update
void Scene::Start()
{
	/* Awake */
	mainCameraObject->Awake();
	mTerrain->Awake();
	ProcessAllObjects([](sptr<GameObject> object) {
		object->Awake();
		});

	for (auto& p : mParticles)
		p->Awake();

	mGameManager->Awake();

	/* Enable & Start */
	mTerrain->OnEnable();
	ProcessAllObjects([](sptr<GameObject> object) {
		object->OnEnable();
		});
	mainCameraObject->OnEnable();
	mGameManager->OnEnable();

	UpdateGridInfo();
}

void Scene::Update()
{
	CheckCollisions();

	mGameManager->Update();
	UpdateObjects();
	mGameManager->LateUpdate();
	for (auto& p : mParticles)
		p->Update();

	mainCameraObject->Update();
	mainCamera->UpdateViewMtx();
	mLight->Update();
	canvas->Update();
	pr->Update();

	UpdateShaderVars();

	PopObjectBuffer();
}

void Scene::CheckCollisions()
{
	for (Grid& grid : mGrids) {
		grid.CheckCollisions();
	}
}

void Scene::UpdateObjects()
{
	ProcessActiveObjects([this](sptr<GridObject> object) {
		if (object->IsActive()) {
			object->Update();
		}
		});

	ProcessActiveObjects([this](sptr<GridObject> object) {
		if (object->IsActive()) {
			object->Animate();
		}
		});

	ProcessActiveObjects([this](sptr<GridObject> object) {
		if (object->IsActive()) {
			object->LateUpdate();
		}
		});
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
int Scene::GetGridIndexFromPos(Vec3 pos) const
{
	pos.x -= mGridStartPoint;
	pos.z -= mGridStartPoint;

	const int gridX = static_cast<int>(pos.x / mGridhWidth);
	const int gridZ = static_cast<int>(pos.z / mGridhWidth);

	return gridZ * mGridCols + gridX;
}


void Scene::ToggleDrawBoundings()
{
	mIsRenderBounds = !mIsRenderBounds;

	ProcessAllObjects([](sptr<GridObject> object) {
		object->ToggleDrawBoundings();
		});
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

sptr<GridObject> Scene::Instantiate(const std::string& modelName, bool enable)
{
	const auto& model = res->Get<MasterModel>(modelName);
	if (!model) {
		return nullptr;
	}

	sptr<GridObject> instance = std::make_shared<GridObject>();
	instance->SetModel(model);
	instance->SetTag(instance->GetTag());
	if (enable) {
		instance->OnEnable();
	}
	mDynamicObjectBuffer.push_back(instance);

	return instance;
}

sptr<ObjectPool> Scene::CreateObjectPool(const std::string& modelName, int maxSize, std::function<void(rsptr<InstObject>)> objectInitFunc)
{
	return CreateObjectPool(res->Get<MasterModel>(modelName), maxSize, objectInitFunc);
}

sptr<ObjectPool> Scene::CreateObjectPool(rsptr<const MasterModel> model, int maxSize, std::function<void(rsptr<InstObject>)> objectInitFunc)
{
	sptr<ObjectPool> pool = mObjectPools.emplace_back(std::make_shared<ObjectPool>(model, maxSize, sizeof(SB_StandardInst)));
	pool->CreateObjects<InstObject>(objectInitFunc);

	return pool;
}

void Scene::ProcessActiveObjects(std::function<void(sptr<GridObject>)> processFunc)
{
	for (auto& object : mStaticObjects) {
		if (object->IsActive()) {
			processFunc(object);
		}
	}

	for (auto& object : mDynamicObjects) {
		if (object->IsActive()) {
			processFunc(object);
		}
	}

	for (auto& object : mObjectPools) {
		object->DoActiveObjects(processFunc);
	}
}

void Scene::ProcessAllObjects(std::function<void(sptr<GridObject>)> processFunc)
{
	for (auto& object : mStaticObjects) {
		processFunc(object);
	}

	for (auto& object : mDynamicObjects) {
		processFunc(object);
	}

	for (auto& object : mObjectPools) {
		object->DoAllObjects(processFunc);
	}
}