#include "stdafx.h"
#include "Object.h"

#include "Mesh.h"
#include "Model.h"
#include "Scene.h"
#include "Collider.h"
#include "ObjectPool.h"

#include "Script_Apache.h"
#include "Script_Gunship.h"

#include "AnimationController.h"


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
		mAnimationController = std::make_shared<AnimationController>(1, animationInfo, this);
	}

	// 모델의 이름에 따라 설정한다.
	switch (Hash(mMasterModel->GetName())) {
	case Hash("Apache"):
		AddComponent<Script_Apache>();
		break;
	case Hash("Gunship"):
		AddComponent<Script_Gunship>();
		break;
	default:
		break;
	}

	// 이 객체의 계층구조를 [mMergedTransform]에 저장한다 (캐싱)
	Transform::MergeTransform(mMergedTransform, this);
}

void GameObject::Animate()
{
	base::Animate();

	if (mAnimationController) {
		mAnimationController->Animate();
	}
}

void GameObject::Render()
{
	if (mAnimationController) {
		mAnimationController->UpdateShaderVariables();
	}
	if (mMasterModel) {
		mMasterModel->Render(this);
	}
}

Transform* GameObject::FindFrame(const std::string& frameName)
{
	if (GetName() == frameName) {
		return this;
	}

	Transform* transform{};
	if (mSibling) {
		if (transform = mSibling->GetObj<GameObject>()->FindFrame(frameName)) {
			return transform;
		}
	}
	if (mChild) {
		if (transform = mChild->GetObj<GameObject>()->FindFrame(frameName)) {
			return transform;
		}
	}

	return nullptr;
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

	UpdateGrid();
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
