#include "stdafx.h"
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

	// ÀÌ °´Ã¼ÀÇ °èÃþ±¸Á¶¸¦ [mMergedTransform]¿¡ ÀúÀåÇÑ´Ù (Ä³½Ì)
	Transform::MergeTransform(mMergedTransform, this);
}

void GameObject::SetModel(const std::string& modelName)
{
	SetModel(res->Get<MasterModel>(modelName));
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
	const float terrainHeight = scene->GetTerrainHeight(pos.x, pos.z);
	pos.y = terrainHeight;

	SetPosition(pos);
}
#pragma endregion





#pragma region GridObject
GridObject::GridObject()
	:
	mCollider(AddComponent<ObjectCollider>().get())
{

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

	scene->RemoveObjectFromGrid(this);
}

void GridObject::RenderBounds()
{
	if (mIsDrawBounding && mCollider) {
		GetComponent<ObjectCollider>()->Render();
	}
}

void GridObject::UpdateGrid()
{
	scene->UpdateObjectGrid(this);
}

void GridObject::RemoveCollider()
{
	RemoveComponent<ObjectCollider>();
	mCollider = nullptr;
}
#pragma endregion





#pragma region InstObject
InstObject::InstObject(ObjectPool* pool, int id)
	:
	mObjectPool(pool),
	mPoolID(id)
{
	
}

void InstObject::OnDestroy()
{
	base::OnDestroy();
	mObjectPool->Return(this);
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

void InstObject::PushFunc(void* structuredBuffer) const
{
	SB_StandardInst* buffer = static_cast<SB_StandardInst*>(structuredBuffer);
	XMStoreFloat4x4(&buffer->LocalTransform, XMMatrixTranspose(_MATRIX(GetWorldTransform())));
}

void InstObject::PushRender()
{
	if (mIsPushed) {
		return;
	}

	mIsPushed = true;
	mObjectPool->PushRender(this);
}
#pragma endregion





#pragma region InstBulletObject
void InstBulletObject::PushFunc(void* structuredBuffer) const
{
	SB_ColorInst* buffer = static_cast<SB_ColorInst*>(structuredBuffer);
	XMStoreFloat4x4(&buffer->LocalTransform, XMMatrixTranspose(_MATRIX(GetWorldTransform())));
	buffer->Color = Vec4(1, 1, 0, 1);
}

void InstBulletObject::UpdateGrid()
{
	scene->UpdateObjectGrid(this, false);
}
#pragma endregion
