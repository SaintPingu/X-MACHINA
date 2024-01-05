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
#include "Instancing.h"

#include "Script_Apache.h"
#include "Script_Gunship.h"




#pragma region GameObjec
rsptr<Texture> GameObject::GetTexture() const
{
	return mMasterModel->GetTexture();
}


void GameObject::SetModel(rsptr<const MasterModel> model)
{
	mMasterModel = model;
	mMasterModel->CopyModelHierarchy(this);

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


void GameObject::Render()
{
	if (mMasterModel) {
		mMasterModel->Render(this);
	}
}

void GameObject::Update()
{
	if (!IsActive()) {
		return;
	}

	base::Update();
}

void GameObject::Enable()
{
	mIsActive = true;

	base::OnEnable();
}

void GameObject::Disable()
{
	base::OnDisable();

	mIsActive = false;
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
	mCollider(AddComponent<ObjectCollider>())
{

}

void GridObject::Enable()
{
	scene->UpdateObjectGrid(this);

	base::OnEnable();
}

void GridObject::Disable()
{
	base::OnDisable();

	scene->RemoveObjectFromGrid(this);
}

void GridObject::RenderBounds()
{
	if (mIsDrawBounding) {
		GetComponent<ObjectCollider>()->Render();
	}
}
#pragma endregion





#pragma region InstObject
void InstObject::SetBuffer(ObjectPool* buffer, int id)
{
	mBuffer = buffer;
	mPoolID = id;

	switch (mType) {
	case ObjectType::DynamicMove:
		mUpdateFunc = [this]() { UpdateDynamic(); };
		break;

	default:
		mUpdateFunc = [this]() { UpdateStatic(); };
		break;
	}
}


void InstObject::OnDestroy()
{
	base::OnDestroy();
	mBuffer->Return(this);
}

void InstObject::Push()
{
	if (mIsPushed) {
		return;
	}

	mIsPushed = true;
	mBuffer->PushObject(this);
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