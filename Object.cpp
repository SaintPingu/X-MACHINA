#include "stdafx.h"
#include "Object.h"
#include "DXGIMgr.h"

#include "Model.h"
#include "Terrain.h"
#include "Mesh.h"
#include "Camera.h"
#include "Scene.h"
#include "Timer.h"
#include "Collider.h"
#include "ObjectPool.h"

#include "Script_Apache.h"
#include "Script_Gunship.h"




#pragma region GameObject
rsptr<Texture> GameObject::GetTexture() const
{
	return mMasterModel->GetTexture();
}


void GameObject::SetModel(rsptr<const MasterModel> model)
{
	mMasterModel = model;
	mMasterModel->CopyModelHierarchy(this);

	// ���� �̸��� ���� �����Ѵ�.
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

	// �� ��ü�� ���������� [mMergedTransform]�� �����Ѵ� (ĳ��)
	Transform::MergeTransform(mMergedTransform, this);
}


void GameObject::Update()
{
	base::Update();
}

void GameObject::Render()
{
	if (mMasterModel) {
		mMasterModel->Render(this);
	}
}

Transform* GameObject::FindFrame(const std::string& frameName)
{
	if (mName == frameName) {
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
	switch (mType) {
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
	SB_StandardInst* data = static_cast<SB_StandardInst*>(structuredBuffer);
	XMStoreFloat4x4(&data->LocalTransform, XMMatrixTranspose(_MATRIX(GetWorldTransform())));
}

void InstObject::PushRender()
{
	if (mIsPushed) {
		return;
	}

	mIsPushed = true;
	mObjectPool->PushRender(this);
}

void InstObject::UpdateStatic()
{
	Pop();
}
void InstObject::UpdateDynamic()
{
	base::Update();
	Pop();
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
