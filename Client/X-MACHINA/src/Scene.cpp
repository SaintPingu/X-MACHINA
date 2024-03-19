#pragma region Include
#include "stdafx.h"
#include "Scene.h"
#include "DXGIMgr.h"
#include "MultipleRenderTarget.h"
#include "FrameResource.h"

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
#include "RootSignature.h"
#include "DescriptorHeap.h"
#include "ObjectPool.h"

#include "Animator.h"
#include "AnimatorController.h"

#include "Script_Player.h"
#include "Script_ExplosiveObject.h"
#include "Script_Billboard.h"
#include "Script_Sprite.h"
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region C/Dtor
namespace {
	constexpr int kGridWidthCount = 20;						// all grid count = n*n
	constexpr Vec3 kBorderPos = Vec3(256, 200, 256);		// center of border
	constexpr Vec3 kBorderExtents = Vec3(1500, 500, 1500);		// extents of border

	constexpr D3D12_PRIMITIVE_TOPOLOGY kObjectPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	constexpr D3D12_PRIMITIVE_TOPOLOGY kUIPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	constexpr D3D12_PRIMITIVE_TOPOLOGY kTerrainPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	constexpr D3D12_PRIMITIVE_TOPOLOGY kBoundsPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
}

Scene::Scene()
	:
	mMapBorder(kBorderPos, kBorderExtents),
	mGridhWidth(static_cast<int>(mMapBorder.Extents.x / kGridWidthCount)),
	mLight(std::make_shared<Light>()),
	mDescriptorHeap(std::make_unique<DescriptorHeap>())
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

sptr<const MasterModel> Scene::GetModel(const std::string& modelName) const
{
	if (!mModels.contains(modelName)) {
		return nullptr;
	}

	return mModels.at(modelName);
}

sptr<Texture> Scene::GetTexture(const std::string& name) const
{
	if (!mTextureMap.contains(name)) {
		return nullptr;
	}

	return mTextureMap.at(name);
}

rsptr<DescriptorHeap> Scene::GetDescHeap() const
{
	assert(mDescriptorHeap);

	return mDescriptorHeap;
}

sptr<AnimatorController> Scene::GetAnimatorController(const std::string& controllerFile) const
{
	if (!mAnimatorControllerMap.contains(controllerFile)) {
		return nullptr;
	}

	return std::make_shared<AnimatorController>(*mAnimatorControllerMap.at(controllerFile));
}

RComPtr<ID3D12RootSignature> Scene::GetGraphicsRootSignature() const
{
	assert(mGraphicsRootSignature);

	return mGraphicsRootSignature->Get();
}

RComPtr<ID3D12RootSignature> Scene::GetComputeRootSignature() const
{
	assert(mComputeRootSignature);

	return mComputeRootSignature->Get();
}

UINT Scene::GetGraphicsRootParamIndex(RootParam param) const
{
	return mGraphicsRootSignature->GetRootParamIndex(param);
}

UINT Scene::GetComputeRootParamIndex(RootParam param) const
{
	return mComputeRootSignature->GetRootParamIndex(param);
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
	constexpr UINT kNum32Bit = 16U;
	cmdList->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, const Vec4x4& data, UINT offset)
{
	constexpr UINT kNum32Bit = 16U;
	cmdList->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, const Vec4& data, UINT offset)
{
	constexpr UINT kNum32Bit = 4U;
	cmdList->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void Scene::SetGraphicsRoot32BitConstants(RootParam param, float data, UINT offset)
{
	constexpr UINT kNum32Bit = 1U;
	cmdList->SetGraphicsRoot32BitConstants(GetGraphicsRootParamIndex(param), kNum32Bit, &data, offset);
}

void Scene::SetGraphicsRootConstantBufferView(RootParam param, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr)
{
	cmdList->SetGraphicsRootConstantBufferView(GetGraphicsRootParamIndex(param), gpuAddr);
}

void Scene::SetGraphicsRootShaderResourceView(RootParam param, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr)
{
	cmdList->SetGraphicsRootShaderResourceView(GetGraphicsRootParamIndex(param), gpuAddr);
}

void Scene::CreateShaderResourceView(RComPtr<ID3D12Resource> resource, DXGI_FORMAT srvFormat)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = srvFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

	mDescriptorHeap->CreateShaderResourceView(resource, &srvDesc);
}

void Scene::CreateShaderResourceView(Texture* texture)
{
	ComPtr<ID3D12Resource> resource = texture->GetResource();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = texture->GetShaderResourceViewDesc();

	mDescriptorHeap->CreateShaderResourceView(resource, &srvDesc);

	texture->SetGpuDescriptorHandle(mDescriptorHeap->GetGPUSrvLastHandle(), mDescriptorHeap->GetGPUSrvLastHandleIndex());
}

void Scene::CreateGraphicsRootSignature()
{
	mGraphicsRootSignature = std::make_shared<GraphicsRootSignature>();

	// 자주 사용되는 것을 앞에 배치할 것. (빠른 메모리 접근)
	mGraphicsRootSignature->Push(RootParam::Object,		D3D12_ROOT_PARAMETER_TYPE_CBV, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
	mGraphicsRootSignature->Push(RootParam::Pass,		D3D12_ROOT_PARAMETER_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
	mGraphicsRootSignature->Push(RootParam::SkinMesh,	D3D12_ROOT_PARAMETER_TYPE_CBV, 2, 0, D3D12_SHADER_VISIBILITY_ALL);
	// GameObjectInfo를 Collider로 변경, 충돌 박스만 그려주는 용도
	mGraphicsRootSignature->Push(RootParam::Collider,	D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 3, 0, D3D12_SHADER_VISIBILITY_ALL, 16);

	// 머티리얼은 space1을 사용하여 t0을 TextureCube와 같이 사용하여도 겹치지 않음
	mGraphicsRootSignature->Push(RootParam::Instancing,	D3D12_ROOT_PARAMETER_TYPE_SRV, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	mGraphicsRootSignature->Push(RootParam::Material,	D3D12_ROOT_PARAMETER_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	
	// TextureCube 형식을 제외한 모든 텍스처들은 Texture2D 배열에 저장된다.
	mGraphicsRootSignature->PushTable(RootParam::SkyBox,	D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	mGraphicsRootSignature->PushTable(RootParam::Texture,	D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, gkMaxTexture, D3D12_SHADER_VISIBILITY_PIXEL);
	

	mGraphicsRootSignature->Create();
}

void Scene::CreateComputeRootSignature()
{
	mComputeRootSignature = std::make_shared<ComputeRootSignature>();

	// 가중치 루트 상수 (b0)
	mComputeRootSignature->Push(RootParam::Weight, D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 0, 0, D3D12_SHADER_VISIBILITY_ALL, 12);

	// 읽기 전용 SRV 서술자 테이블 (t0)
	mComputeRootSignature->PushTable(RootParam::Read, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 0, 1, D3D12_SHADER_VISIBILITY_ALL);	
	
	// 읽기 전용 SRV 서술자 테이블 (t1, t2)
	mComputeRootSignature->PushTable(RootParam::LUT0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
	mComputeRootSignature->PushTable(RootParam::LUT1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 1, D3D12_SHADER_VISIBILITY_ALL);

	// 쓰기 전용 UAV 서술자 테이블 (u0)
	mComputeRootSignature->PushTable(RootParam::Write, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 0, 1, D3D12_SHADER_VISIBILITY_ALL);

	mComputeRootSignature->Create();
}

void Scene::UpdateShaderVars()
{
	// TODO : AnimateMaterials(); 
	UpdateMainPassCB();
	UpdateMaterialBuffer();
}

void Scene::UpdateMainPassCB()
{
	static float timeElapsed{};
	timeElapsed += DeltaTime();

	PassConstants passConstants;
	passConstants.MtxView	= XMMatrixTranspose(_MATRIX(mainCamera->GetViewMtx()));
	passConstants.MtxProj	= XMMatrixTranspose(_MATRIX(mainCamera->GetProjMtx()));
	passConstants.EyeW		= mainCamera->GetPosition();
	passConstants.DeltaTime = timeElapsed;
	passConstants.RT1_TextureIndex = dxgi->GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::Texture)->GetGpuDescriptorHandleIndex();
	passConstants.RT2_UIIndex = dxgi->GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::UI)->GetGpuDescriptorHandleIndex();
	passConstants.RT3_NormalIndex = dxgi->GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::Normal)->GetGpuDescriptorHandleIndex();
	passConstants.RT4_DepthIndex = dxgi->GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::Depth)->GetGpuDescriptorHandleIndex();
	passConstants.RT5_DistanceIndex = dxgi->GetMRT(GroupType::GBuffer)->GetTexture(GBuffer::Distance)->GetGpuDescriptorHandleIndex();
	memcpy(&passConstants.Lights, mLight->GetSceneLights().get(), sizeof(passConstants.Lights)); // 조명 정보 가져오기
	
	frmResMgr->CopyData(passConstants);
}

void Scene::UpdateMaterialBuffer()
{
	// TODO : Update only if the numFramesDirty is greater than 0
	for (auto& model : mModels) {
		model.second->GetMesh()->UpdateMaterialBuffer();
	}
}

void Scene::CreateCbvSrvDescriptorHeaps(int cbvCount, int srvCount, int uavCount)
{
	mDescriptorHeap->Create(cbvCount, srvCount, uavCount);
}
#pragma endregion





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Build
void Scene::BuildObjects()
{
	CreateGraphicsRootSignature();
	CreateComputeRootSignature();

	assert(mGraphicsRootSignature);
	CreateCbvSrvDescriptorHeaps(0, 1024, 256);

	// load materials
	mTextureMap = FileIO::LoadTextures("Import/Textures/");

	// load canvas (UI)
	canvas->Init();

	// load models
	LoadAnimationClips();
	LoadAnimatorControllers();
	LoadSceneObjects("Import/Scene.bin");
	LoadModels();

	// build settings
	BuildPlayers();
	BuildTerrain();

	// build 
 	BuildShaders();
	// build static meshes
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
	BuildDeferredShader();
	BuildForwardShader();
}

void Scene::BuildForwardShader()
{
	ShaderType shaderType = ShaderType::Forward;

	mWaterShader = std::make_shared<WaterShader>();
	mWaterShader->Create(shaderType);

	mBoundingShader = std::make_shared<WireShader>();
	mBoundingShader->Create(shaderType);

	mBillboardShader = std::make_shared<BillboardShader>();
	mBillboardShader->Create(shaderType);

	mSpriteShader = std::make_shared<SpriteShader>();
	mSpriteShader->Create(shaderType);

	mSkinnedMeshShader = std::make_shared<SkinMeshShader>();
	mSkinnedMeshShader->Create(shaderType);

	mFinalShader = std::make_shared<FinalShader>();
	mFinalShader->Create(shaderType, DXGI_FORMAT_D32_FLOAT);
}

void Scene::BuildDeferredShader()
{
	ShaderType shaderType = ShaderType::Deferred;

	mGlobalShader = std::make_shared<DeferredShader>();
	mGlobalShader->Create(shaderType);

	mInstShader = std::make_shared<ObjectInstShader>();
	mInstShader->Create(shaderType);

	mTransparentShader = std::make_shared<TransparentShader>();
	mTransparentShader->Create(shaderType);

	mBulletShader = std::make_shared<ColorInstShader>();
	mBulletShader->Create(shaderType);
}

void Scene::BuildPlayers()
{
	mPlayers.reserve(1);
	sptr<GridObject> airplanePlayer = std::make_shared<GridObject>();
	airplanePlayer->AddComponent<Script_GroundPlayer>()->CreateBullets(GetModel("tank_bullet"));
	airplanePlayer->SetModel(GetModel("EliteTrooper"));

	mPlayers.push_back(airplanePlayer);

	mPlayer = mPlayers.front();
	mPlayerScript = mPlayer->GetComponent<Script_GroundPlayer>();
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

			model = FileIO::LoadGeometryFromFile("Import/Meshes/" + meshName + ".bin");
			mModels.insert(std::make_pair(meshName, model));

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

void Scene::LoadModels()
{
	const std::string rootFolder = "Import/Meshes/";
	sptr<MasterModel> model;
	for (const auto& modelFile : std::filesystem::directory_iterator(rootFolder)) {
		const std::string fileName = modelFile.path().filename().string();
		const std::string modelName = FileIO::RemoveExtension(fileName);

		if (mModels.contains(modelName)) {
			continue;
		}

		model = FileIO::LoadGeometryFromFile(rootFolder + fileName);
		if (fileName.substr(0, 6) == "sprite") {
			model->SetSprite();
		}
		mModels.insert(std::make_pair(modelName, model));
	}
}

void Scene::LoadAnimationClips()
{
	const std::string rootFolder = "Import/AnimationClips/";
	for (const auto& clipFolder : std::filesystem::directory_iterator(rootFolder)) {
		std::string clipFolderName = clipFolder.path().filename().string();

		for (const auto& file : std::filesystem::directory_iterator(rootFolder + clipFolderName + '/')) {
			std::string fileName = file.path().filename().string();
			sptr<const AnimationClip> clip = FileIO::LoadAnimationClip(clipFolder.path().string() + '/' + fileName);

			FileIO::RemoveExtension(fileName);
			mAnimationClipMap[clipFolderName].insert(std::make_pair(fileName, clip));
		}
	}
}

void Scene::LoadAnimatorControllers()
{
	const std::string rootFolder = "Import/AnimatorControllers/";
	for (const auto& file : std::filesystem::directory_iterator(rootFolder)) {
		const std::string fileName = file.path().filename().string();
		mAnimatorControllerMap.insert(std::make_pair(FileIO::RemoveExtension(fileName), FileIO::LoadAnimatorController(rootFolder + fileName)));
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

void Scene::OnPrepareRender()
{
	cmdList->SetGraphicsRootSignature(GetGraphicsRootSignature().Get());

	mainCamera->SetViewportsAndScissorRects();

	mDescriptorHeap->Set();

	// 모든 Pass, Material, Texture는 한 프레임에 한 번만 설정한다.
	cmdList->SetGraphicsRootConstantBufferView(GetGraphicsRootParamIndex(RootParam::Pass), frmResMgr->GetPassCBGpuAddr());
	cmdList->SetGraphicsRootShaderResourceView(GetGraphicsRootParamIndex(RootParam::Material), frmResMgr->GetMatBufferGpuAddr(0));
	cmdList->SetGraphicsRootDescriptorTable(GetGraphicsRootParamIndex(RootParam::Texture), mDescriptorHeap->GetGPUHandle());
}

void Scene::RenderShadow()
{
	// 추후에 그림자를 렌더링하는 함수
	// 그림자용 카메라를 기준으로 모든 오브젝트의 깊이 값을 렌더링한다. 
	// 깊이 값이 저장된 그림자 맵이 생성된 상태일 것이다.
}

void Scene::RenderDeferred()
{
	// 추후에 디퍼드 렌더링 구현시 물체의 깊이 정보를 저장하고 조명 계산에 활용해야 한다.
	// 현재는 MRT에서 조명을 처리하고 있는데 이후에 Final이나 Canvas에서 전달 받은 메쉬에 대해서만 조명 처리를 해야한다.
	// Directional 조명은 화면 전체 크기의 사각형 볼륨 메쉬, Point, Spot 조명은 조명의 길이의 반지름을 가진 구 볼륨 메쉬를 가진다.
	// 블렌딩을 사용한 투명(반투명) 객체는 깊이 버퍼에 깊이 값을 저장할 수 없기 때문에 포워드 렌더링에서 처리해야만 한다.
	// 따라서 deferred -> light(volume mesh) -> final(canvas) -> forward 순서로 렌더링을 처리해야 한다.
#pragma region PrepareRender
	mRenderedObjects.clear();
	mSkinMeshObjects.clear();
	mTransparentObjects.clear();
	mBillboardObjects.clear();
	OnPrepareRender();
#pragma endregion

	mGlobalShader->Set();

	cmdList->IASetPrimitiveTopology(kObjectPrimitiveTopology);
	RenderGridObjects();	
	RenderSkinMeshObjects();
	RenderEnvironments();	
	RenderInstanceObjects();
	RenderBullets();

	cmdList->IASetPrimitiveTopology(kTerrainPrimitiveTopology);
	RenderTerrain();
}

void Scene::RenderLights()
{
	// 포지션, 노말, 컬러 등의 모든 결과 텍스처를 샘플링하여 조명 처리를 진행한다.
	// 이때 그림자 맵도 샘플링하고 현재 픽셀 값이 그림자 맵보다 큰 경우에 어둡게 렌더링한다.
	// 결과 값인 diffuse와 specular를 각각의 타겟 텍스처에 출력한다.
}

void Scene::RenderFinal()
{
	// 조명에서 출력한 diffuse와 specular를 결합하여 최종 색상을 렌더링한다.
	mFinalShader->Set();

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(6, 1, 0, 0);
}

void Scene::RenderForward()
{
	// 렌더 타겟 텍스처에 렌더링하는 것이 아닌 후면 버퍼에 바로 렌더링한다.
	// forward 쉐이더를 사용하는 오브젝트가 조명을 사용하고자 한다면 
	// 바로 해당 쉐이더에서 lighting 연산을 수행한다.
	RenderFXObjects(); 
	RenderBillboards();

	cmdList->IASetPrimitiveTopology(kObjectPrimitiveTopology);
	RenderTransparentObjects(mTransparentObjects); 
	RenderSkyBox();

	if (RenderBounds(mRenderedObjects)) {
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

void Scene::RenderTransparentObjects(const std::set<GridObject*>& transparentObjects)
{
	mTransparentShader->Set();
	for (auto& object : transparentObjects) {
		object->Render();
	}
	if (mWater) {
		mWaterShader->Set();
		mWater->Render();
	}
}


void Scene::RenderSkyBox()
{
	mSkyBox->Render();
}


void Scene::RenderGridObjects()
{
	for (const auto& grid : mGrids) {
		if (grid.Empty()) {
			continue;
		}

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

void Scene::RenderSkinMeshObjects()
{
	mSkinnedMeshShader->Set();
	for (auto& object : mSkinMeshObjects) {
		object->Render();
	}
}

void Scene::RenderInstanceObjects()
{
	mInstShader->Set();
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
	mBulletShader->Set();
	for (auto& player : mPlayers) {
		if (player->IsActive()) {
			//player->GetComponent<Script_AirplanePlayer>()->RenderBullets();
		}
	}
}


bool Scene::RenderBounds(const std::set<GridObject*>& renderedObjects)
{
	if (!mIsRenderBounds || !mBoundingShader) {
		return false;
	}

	cmdList->IASetPrimitiveTopology(kBoundsPrimitiveTopology);

	mBoundingShader->Set();
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
	mBillboardShader->Set();
	for (auto& object : mBillboardObjects) {
		object->Render();
	}
	mSpriteShader->Set();
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
	UpdateLights();
	UpdateCamera();
	canvas->Update();

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
			break;
		case VK_DELETE:
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
	const auto& model = GetModel(modelName);
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