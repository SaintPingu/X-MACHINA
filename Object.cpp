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

#include "Script_Apache.h"
#include "Script_Gunship.h"



#pragma region ModelObject
ModelObject::ModelObject()
{
	mIsDrawBounding = false;
}

/* DirectX */
void ModelObject::UpdateShaderVars() const
{
	scene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrixTranspose(_MATRIX(GetWorldTransform())), 0);
}

void ModelObject::ReleaseUploadBuffers()
{
	base::ReleaseUploadBuffers();
}


/* Others */
void ModelObject::ToggleDrawBoundings()
{
	mIsDrawBounding = !mIsDrawBounding;

	if (mSibling) {
		mSibling->GetObj<GameObject>()->ToggleDrawBoundings();
	}
	if (mChild) {
		mChild->GetObj<GameObject>()->ToggleDrawBoundings();
	}
}

Transform* ModelObject::FindFrame(const std::string& frameName)
{
	if (mName == frameName) {
		return this;
	}

	Transform* transform{};
	if (mSibling) {
		if (transform = mSibling->GetObj<ModelObject>()->FindFrame(frameName)) {
			return transform;
		}
	}
	if (mChild) {
		if (transform = mChild->GetObj<ModelObject>()->FindFrame(frameName)) {
			return transform;
		}
	}

	return nullptr;
}

ModelObject* ModelObject::FindObject(const std::string& frameName)
{
	if (mName == frameName) {
		return this;
	}

	if (mSibling) {
		ModelObject* object = mSibling->GetObj<ModelObject>()->FindObject(frameName);
		if (object) {
			return object;
		}
	}
	if (mChild) {
		ModelObject* object = mChild->GetObj<ModelObject>()->FindObject(frameName);
		if (object) {
			return object;
		}
	}

	return nullptr;
}

void ModelObject::Update()
{
	Object::Update();
}
#pragma endregion





#pragma region GameObject
GameObject::GameObject() : ModelObject()
{
	AddComponent<ObjectCollider>();
}


rsptr<Texture> GameObject::GetTexture() const
{
	return mModel->GetTexture();
}


void GameObject::SetModel(rsptr<const MasterModel> model)
{
	mModel = model;
	mModel->CopyModelHierarchy(this);

	if (mModel->GetName() == "Apache") {
		AddComponent<Script_Apache>();
	}
	else if (mModel->GetName() == "Gunship") {
		AddComponent<Script_Gunship>();
	}

	Transform::MergeTransform(mMergedTransform, this);
}


void GameObject::Render()
{
	if (mModel) {
		mModel->Render(this);
	}
}

void GameObject::RenderBounds()
{
	if (mIsDrawBounding) {
		GetComponent<ObjectCollider>()->Render();
	}
}

void GameObject::Update()
{
	if (!IsActive()) {
		return;
	}

	if (!mIsFlyable) {
		TiltToGround();
	}

	base::Update();
}

void GameObject::Enable(bool isUpdateObjectGrid)
{
	mIsActive = true;

	if (isUpdateObjectGrid) {
		scene->UpdateObjectGrid(this);
	}
}

void GameObject::Disable(bool isUpdateObjectGrid)
{
	mIsActive = false;

	if (isUpdateObjectGrid) {
		scene->RemoveObjectFromGrid(this);
	}
}

// 객체의 바닥 중심, 앞, 뒤, 좌, 우를 기준으로 하여 지면에 붙도록 한다.
void GameObject::AttachToGround()
{
	Vec3 pos                  = GetPosition();
	const float terrainHeight = scene->GetTerrainHeight(pos.x, pos.z);
	pos.y                     = terrainHeight;

	SetPosition(pos);
}


void GameObject::TiltToGround()
{
	AttachToGround();

	auto& obbList = GetComponent<ObjectCollider>()->GetOBBList();
	if (obbList.size() > 0) {
		// 기울임 각도 계산
		// OBB 모서리 아래 4개 점 구하기
		std::vector<Vec3> corners(8);
		obbList.front()->GetCorners(corners.data());
		const Vec3 leftTop     = corners[0];
		const Vec3 rightTop    = corners[1];
		const Vec3 leftBottom  = corners[4];
		const Vec3 rightBottom = corners[5];

		// weight points
		const Vec3 center = GetPosition();
		const Vec3 front  = Vector3::Add(rightTop, Vector3::Subtract(leftTop, rightTop), 0.5f);
		const Vec3 back   = Vector3::Add(rightBottom, Vector3::Subtract(leftBottom, rightBottom), 0.5f);
		const Vec3 left   = Vector3::Add(center, Vector3::Subtract(leftTop, rightTop), 0.5f);
		const Vec3 right  = Vector3::Add(center, Vector3::Subtract(rightTop, leftTop), 0.5f);

		// 각 지점에 대해 높이 차이 계산
		auto GetHeight = [&](const Vec3& pos) { return pos.y - scene->GetTerrainHeight(pos.x, pos.z); };

		const float heightFront = GetHeight(front);
		const float heightBack  = -GetHeight(back);
		const float heightLeft  = GetHeight(left);
		const float heightRight = -GetHeight(right);

		// 앞,뒤 / 좌,우로 높이차가 가장 큰 값에 대해 회전
		const float yaw  = max(heightFront, heightBack);
		const float roll = max(heightLeft, heightRight);

		// 지면에 닿도록 각도로 회전
		if (!Math::IsZero(yaw) || !Math::IsZero(roll)) {
			Rotate(80.f * yaw * DeltaTime(), 0.f, 0.f);
			Rotate(0.f, 0.f, 80.f * roll * DeltaTime());
		}
	}
}
#pragma endregion





#pragma region InstancinObject
void InstancinObject::SetBuffer(rsptr<ObjectInstanceBuffer> buffer)
{
	SetInstancing();
	mBuffer = buffer;

	switch (mType) {
	case ObjectType::DynamicMove:
		mUpdateFunc = [this]() { UpdateDynamic(); };
		break;

	default:
		mUpdateFunc = [this]() { UpdateStatic(); };
		break;
	}
}

void InstancinObject::Render()
{
	Push();
}

void InstancinObject::Push()
{
	if (mIsPushed) {
		return;
	}

	mIsPushed = true;
	mBuffer->PushObject(this);
}

void InstancinObject::UpdateStatic()
{
	Reset();
}
void InstancinObject::UpdateDynamic()
{
	base::Update();
	Reset();
}
#pragma endregion





#pragma region ObjectInstanceBuffer
void ObjectInstanceBuffer::SetModel(rsptr<const MasterModel> model)
{
	mModel = model;
	Transform::MergeTransform(mMergedTransform, mModel.get());
	mMergedTransform.erase(mMergedTransform.begin());
}

void ObjectInstanceBuffer::CreateShaderVars(UINT objectCount)
{
	mObjectCnt = objectCount;
	D3DUtil::CreateBufferResource(NULL, sizeof(*mMap_Buff) * mObjectCnt, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, mInstBuff);
	mInstBuff->Map(0, NULL, (void**)&mMap_Buff);
}

void ObjectInstanceBuffer::UpdateShaderVars() const
{
	cmdList->SetGraphicsRootShaderResourceView(scene->GetRootParamIndex(RootParam::Instancing), mInstBuff->GetGPUVirtualAddress());
}

void ObjectInstanceBuffer::PushObject(InstancinObject* object)
{
	assert(mCurrBuffIdx < mObjectCnt);

	XMStoreFloat4x4(&mMap_Buff[mCurrBuffIdx++].LocalTransform, XMMatrixTranspose(_MATRIX(object->GetWorldTransform())));
}

void ObjectInstanceBuffer::Render()
{
	if (mModel) {
		mModel->Render(this);
	}

	ResetBuffer();
}
#pragma endregion