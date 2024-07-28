#include "EnginePch.h"
#include "Object.h"

#include "DXGIMgr.h"
#include "FrameResource.h"
#include "Model.h"
#include "BattleScene.h"
#include "Component/Collider.h"
#include "ObjectPool.h"
#include "MultipleRenderTarget.h"

#include "Animator.h"
#include "Texture.h"
#include "Component/Camera.h"


#pragma region GameObject
rsptr<Texture> GameObject::GetTexture() const
{
	return mMasterModel->GetTexture();
}


void GameObject::SetModel(rsptr<const MasterModel> model)
{
	assert(model);
	mMasterModel = model;
	mMasterModel->CopyModelHierarchy(this);
	mIsSkinMesh = mMasterModel->IsSkinMesh();
	mObjectCB.IsSkinMesh = mIsSkinMesh;

	// 최상위 부모의 mUseObjCB가 true여야 객체 파괴시 오브젝트 인덱스 반환 가능
	SetUseObjCB(true);

	sptr<const AnimationLoadInfo> animationInfo = model->GetAnimationInfo();
	if (animationInfo) {
		mAnimator = std::make_shared<Animator>(animationInfo, this);
	}

	// 이 객체의 계층구조를 [mMergedTransform]에 저장한다 (캐싱)
	GameObject::MergeTransform(mMergedTransform, this);
}


void GameObject::SetModel(const std::string& modelName)
{
	SetModel(RESOURCE<MasterModel>(modelName));
}

void GameObject::Render()
{
	if (mAnimator) {
		mAnimator->UpdateShaderVariables();
	}
	if (mMasterModel) {
		mMasterModel->Render(this);
	}
}

void GameObject::Animate()
{
	base::Animate();

	if (mAnimator) {
		mAnimator->Animate();
	}
}


void GameObject::AttachToGround()
{
	Vec3 pos = GetPosition();
	const float terrainHeight = BattleScene::I->GetTerrainHeight(pos.x, pos.z);
	pos.y = terrainHeight;

	SetPosition(pos);
}

void GameObject::MergeTransform(std::vector<Transform*>& out, GameObject* transform)
{
	if (transform->HasMesh()) {
		out.emplace_back(transform);
	}

	if (transform->mSibling) {
		GameObject::MergeTransform(out, transform->mSibling->GetObj<GameObject>());
	}
	if (transform->mChild) {
		GameObject::MergeTransform(out, transform->mChild->GetObj<GameObject>());
	}
}

#pragma endregion





#pragma region GridObject
GridObject::GridObject()
{

}

void GridObject::ResetCollider()
{
	mCollider = GetComponent<ObjectCollider>().get();
	assert(mCollider);
	ResetColliderColor();
}

void GridObject::SetTag(ObjectTag tag)
{
	ObjectTag beforeTag = GetTag();

	base::SetTag(tag);

	BattleScene::I->UpdateTag(this, beforeTag);
}

void GridObject::Awake()
{
	mCollider = AddComponent<ObjectCollider>().get();
	assert(mCollider);
	ResetColliderColor();
	base::Awake();

	const auto& collider = GetComponent<ObjectCollider>();
	if (collider) {
		mCollider = collider.get();
	}
	else {
		mCollider = nullptr;
	}
}

void GridObject::Update()
{
	base::Update();

	if (IsActive()) {
		UpdateGrid();
	}
}

void GridObject::OnEnable()
{
	base::OnEnable();

	UpdateGrid();
}

void GridObject::OnDisable()
{
	base::OnDisable();

	BattleScene::I->RemoveObjectFromGrid(this);
}

void GridObject::OnDestroy()
{
	base::OnDestroy();
	
	BattleScene::I->RemoveDynamicObject(this);
}

void GridObject::ResetColliderColor()
{
	if (mCollider) {
		Vec3 color;
		switch (GetTag()) {
		case ObjectTag::Player:
			color = Vec3(0, 1, 0);
			break;
		case ObjectTag::Enemy:
			color = Vec3(1, 0, 0);
			break;
		case ObjectTag::Bound:
		case ObjectTag::Building:
		case ObjectTag::DissolveBuilding:
			color = Vec3(1, 1, 0);
			break;
		default:
			color = Vec3(0.2f, 0.2f, 0.2f);
			break;
		}

		mCollider->SetBoundColor(color);
	}
}

void GridObject::RenderBounds()
{
	if (mCollider) {
		GetComponent<ObjectCollider>()->Render();
	}
}

void GridObject::UpdateGrid()
{
	BattleScene::I->UpdateObjectGrid(this);
}

#pragma endregion





#pragma region InstObject
InstObject::InstObject(ObjectPool* pool, int id)
	:
	mObjectPool(pool),
	mPoolID(id)
{
	
}

void InstObject::SetUpdateFunc()
{
	switch (GetType()) {
	case ObjectType::DynamicMove:
		mUpdateFunc = [this]() { UpdateDynamic(); };
		break;

	default:
		mUpdateFunc = [this]() { UpdateStatic(); };
		break;
	}
}

void InstObject::PushFunc(int buffIdx, UploadBuffer<InstanceData>* buffer) const
{
	InstanceData instData;
	instData.MtxWorld = GetWorldTransform().Transpose();

	buffer->CopyData(buffIdx, instData);
}

void InstObject::Return()
{
	mObjectPool->Return(this);
}

void InstObject::PushRender()
{
	mObjectPool->PushRender(this);
}
#pragma endregion





#pragma region InstBulletObject
void InstBulletObject::UpdateGrid()
{
	BattleScene::I->UpdateObjectGrid(this, false);
}
#pragma endregion





#pragma region DynamicEnvironmentObject
UINT DynamicEnvironmentMappingManager::AddObject(Object* object)
{
	if (!mDynamicEnvironmentObjectMap.contains(object)) {
		for (const auto& mrt : mMRTs) {

			bool isContain = false;
			for (const auto& map : mDynamicEnvironmentObjectMap) {
				if (mrt.get() == map.second) {
					isContain = true;
				}
			}

			if (!isContain) {
				mDynamicEnvironmentObjectMap.insert({ object, mrt.get() });
				object->mObjectCB.UseRefract = true;
				object->mObjectCB.DynamicEnvironmentMapIndex = mrt->GetTexture(0)->GetSrvIdx();
				return mrt->GetTexture(0)->GetSrvIdx();
			}
		}
	}

	return -1;
}

void DynamicEnvironmentMappingManager::RemoveObject(Object* object)
{
	mDynamicEnvironmentObjectMap.erase(object);
	object->mObjectCB.UseRefract = false;
	object->mObjectCB.DynamicEnvironmentMapIndex = 0;
}

void DynamicEnvironmentMappingManager::Init()
{
	for (int i = 0; i < mkMaxMRTCount; ++i) {
		// create depth stencil buffer
		std::string dsName = "DynamicEnvironmentDs_" + std::to_string(i);
		sptr<Texture> depthStencilBuffer = ResourceMgr::I->CreateTexture(dsName, 512, 512,
			DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_DEPTH_WRITE, Vec4{ 1.f });
		DXGIMgr::I->CreateDepthStencilView(depthStencilBuffer.get());

		// create mrt
		std::vector<RenderTarget> rts(6);
		std::string rtName = "DynamicEnvironmentRT_" + std::to_string(i);
		sptr<Texture> dynamicCubeMap = ResourceMgr::I->CreateTexture(rtName, 512, 512, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, Vec4{}, D3DResource::TextureCube);

		for (auto& rt : rts) {
			rt.Target = dynamicCubeMap;
		}

		mMRTs[i] = std::make_shared<MultipleRenderTarget>();
		mMRTs[i]->Create(GroupType::DynamicEnvironment, std::move(rts), depthStencilBuffer);
	}

	BuildCubeFaceCamera();
}

void DynamicEnvironmentMappingManager::UpdatePassCB(sptr<Camera> camera, UINT index)
{
	Matrix proj = camera->GetProjMtx();
	PassConstants passCB;
	passCB.MtxView = camera->GetViewMtx().Transpose();
	passCB.MtxProj = camera->GetProjMtx().Transpose();
	passCB.MtxInvProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	passCB.MtxNoLagView = camera->GetNoLagViewtx().Transpose();
	passCB.CameraPos = camera->GetPosition();
	passCB.CameraRight = camera->GetRight();
	passCB.CameraUp = camera->GetUp();
	passCB.FrameBufferWidth = DXGIMgr::I->GetWindowWidth();
	passCB.FrameBufferHeight = DXGIMgr::I->GetWindowHeight();
	passCB.GlobalAmbient = Vec4(0.4f, 0.4f, 0.4f, 1.f);
	passCB.FilterOption = DXGIMgr::I->GetFilterOption();
	passCB.SkyBoxIndex = 1;

	FRAME_RESOURCE_MGR->CopyData(2 + index, passCB);
}

void DynamicEnvironmentMappingManager::Render(const std::set<GridObject*>& objects)
{
	int cnt{};
	for (auto& [object, mrt] : mDynamicEnvironmentObjectMap) {
		auto& cameras = mCameras.at(cnt++);

		for (int i = 0; i < 6; ++i) {
			mrt->ClearRenderTargetView(i, 1.f);
			mrt->OMSetRenderTargets(1, i);

			cameras[i]->SetPosition(object->GetPosition() + Vec3{0.f, 3.f, 0.f});

			auto& cameraScript = cameras[i]->GetCamera();
			cameraScript->UpdateViewMtx();
			UpdatePassCB(cameraScript, i);

			CMD_LIST->SetGraphicsRootConstantBufferView(DXGIMgr::I->GetGraphicsRootParamIndex(RootParam::Pass), FRAME_RESOURCE_MGR->GetPassCBGpuAddr(2 + i));

			BattleScene::I->RenderDynamicEnvironmentMappingObjects();

			mrt->WaitTargetToResource(i);
		}
	}
}

void DynamicEnvironmentMappingManager::BuildCubeFaceCamera()
{
	const Vec3 center{ 0.f };

	const Vec3 targets[6] =
	{
		Vec3(1.0f, 0.f, 0.f), 
		Vec3(-1.0f, 0.f, 0.f),
		Vec3(0.f, 1.0f, 0.f), 
		Vec3(0.f, -1.0f, 0.f),
		Vec3(0.f, 0.f, 1.0f), 
		Vec3(0.f, 0.f, -1.0f) 
	};

	const Vec3 ups[6] =
	{
		Vec3(0.0f, 1.0f, 0.0f), 
		Vec3(0.0f, 1.0f, 0.0f), 
		Vec3(0.0f, 0.0f, -1.0f),
		Vec3(0.0f, 0.0f, +1.0f),
		Vec3(0.0f, 1.0f, 0.0f),	
		Vec3(0.0f, 1.0f, 0.0f)	
	};

	for (int i = 0; i < mkMaxMRTCount; ++i) {
		auto& cameras = mCameras[i];
		
		for (int i = 0; i < 6; ++i) {
			cameras[i] = std::make_shared<CameraObject>();
			cameras[i]->Awake();

			cameras[i]->LookAt(targets[i], ups[i]);
			auto& cameraScript = cameras[i]->GetCamera();
			cameraScript->SetLens(0.5f * XM_PI, 1.0f, 0.1f, 1000.0f);
		}
	}
}
#pragma endregion


