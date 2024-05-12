#include "EnginePch.h"
#include "Object.h"

#include "DXGIMgr.h"
#include "FrameResource.h"
#include "Model.h"
#include "Scene.h"
#include "Component/Collider.h"
#include "ObjectPool.h"

#include "Animator.h"


#pragma region GameObject
rsptr<Texture> GameObject::GetTexture() const
{
	return mMasterModel->GetTexture();
}


void GameObject::SetModel(rsptr<const MasterModel> model)
{
	mMasterModel = model;
	mMasterModel->CopyModelHierarchy(this);
	
	sptr<const AnimationLoadInfo> animationInfo = model->GetAnimationInfo();
	if (animationInfo) {
		mIsSkinMesh = true;
		mAnimator = std::make_shared<Animator>(animationInfo, this);
	}

	// 이 객체의 계층구조를 [mMergedTransform]에 저장한다 (캐싱)
	Transform::MergeTransform(mMergedTransform, this);
}

void GameObject::SetModel(const std::string& modelName)
{
	SetModel(RESOURCE<MasterModel>(modelName));
}

void GameObject::Animate()
{
	base::Animate();

	if (mAnimator) {
		mAnimator->Animate();
	}
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


void GameObject::AttachToGround()
{
	Vec3 pos = GetPosition();
	const float terrainHeight = Scene::I->GetTerrainHeight(pos.x, pos.z);
	pos.y = terrainHeight;

	SetPosition(pos);
}
#pragma endregion





#pragma region GridObject
GridObject::GridObject()
{
}

void GridObject::Awake()
{
	AddComponent<ObjectCollider>();
	base::Awake();

	const auto& collider = GetComponent<ObjectCollider>();
	if (collider) {
		mCollider = collider.get();
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

	Scene::I->RemoveObjectFromGrid(this);
}

void GridObject::OnDestroy()
{
	base::OnDestroy();
	
	Scene::I->RemoveDynamicObject(this);
}

void GridObject::RenderBounds()
{
	if (mIsDrawBounding && mCollider) {
		GetComponent<ObjectCollider>()->Render();
	}
}

void GridObject::UpdateGrid()
{
	Scene::I->UpdateObjectGrid(this);
}


void GridObject::ResetCollider()
{
	mCollider = AddComponent<ObjectCollider>().get();
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
	Scene::I->UpdateObjectGrid(this, false);
}
#pragma endregion