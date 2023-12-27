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













// [ ModelObject ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//===== (ModelObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////
ModelObject::ModelObject()
{
	
}

ModelObject::~ModelObject()
{

}

//===== (ModelObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////



//===== (ModelObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Setter ] /////

//////////////////* Position *//////////////////
void ModelObject::SetPosition(const Vec3& pos)
{
	ModelObject::SetPosition(pos.x, pos.y, pos.z);
}

void ModelObject::SetPosition(const XMVECTOR& pos)
{
	Transform::SetPosition(pos);
	//UpdateBoundingBox();
}

void ModelObject::SetPosition(float x, float y, float z)
{
	Transform::SetPosition(x, y, z);
	auto& collider = GetComponent<ObjectCollider>();
	if (collider) {
		collider->Update();
	}
}


//////////////////* Others *//////////////////


//===== (ModelObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

//////////////////* DirectX *//////////////////
void ModelObject::CreateShaderVariables()
{

}

void ModelObject::UpdateShaderVariables() const
{
	crntScene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrixTranspose(XMLoadFloat4x4(&GetWorldTransform())), 0);
}


void ModelObject::ReleaseShaderVariables()
{

}

void ModelObject::ReleaseUploadBuffers()
{
	Object::ReleaseUploadBuffers();
}




//////////////////* Movement *//////////////////
void ModelObject::Translate(const Vec3& translation)
{
	Transform::Translate(translation);
}

void ModelObject::Translate(const Vec3& direction, float distance)
{
	ModelObject::Translate(Vector3::ScalarProduct(direction, distance));
}

void ModelObject::Translate(float x, float y, float z)
{
	ModelObject::Translate(Vec3(x, y, z));
}

void ModelObject::Rotate(float pitch, float yaw, float roll)
{
	Transform::Rotate(pitch, yaw, roll);
}

void ModelObject::Rotate(const Vec3& axis, float angle)
{
	Transform::Rotate(axis, angle);
}


//////////////////* Transform *//////////////////
void ModelObject::SetWorldTransform(const Vec4x4& transform)
{
	Transform::SetWorldTransform(transform);
}

void ModelObject::SetTransform(const Vec4x4& transform)
{
	Transform::SetTransform(transform);
}


//////////////////* Others *//////////////////
void ModelObject::Update()
{
	Object::Update();
}




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



//////////////////* Lights *//////////////////
//void ModelObject::SetLightRange(LIGHT_RANGE lightRange)
//{
//	mLightRange = lightRange;
//}
//
//void ModelObject::SetAllLight(const LIGHT* light)
//{
//	assert(light);
//	assert(mLightRange.lights);
//
//	for (size_t i = mLightRange.begin; i < mLightRange.end; ++i) {
//		mLightRange.Get(i) = *light;
//		mLightRange.Get(i).mIsEnable = true;
//	}
//}



// [ GameObject ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//===== (GameObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////

GameObject::GameObject() : ModelObject()
{
	AddComponent<ObjectCollider>();
}

GameObject::~GameObject()
{

}



//===== (GameObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////

bool GameObject::IsTransparent() const
{
	return mLayer == ObjectLayer::Transparent;
}

rsptr<Texture> GameObject::GetTexture() const
{
	return mModel->GetTexture();
}

//===== (GameObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Setter ] /////

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

void GameObject::SetFlyable(bool isFlyable)
{
	mIsFlyable = isFlyable;
}

//===== (GameObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

//////////////////* DirectX *//////////////////
void GameObject::UpdateShaderVariablesSprite()
{

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



//////////////////* Movement *//////////////////
void GameObject::Update()
{
	if (!IsActive()) {
		return;
	}

	if (!mIsFlyable) {
		TiltToGround();
	}

	ModelObject::Update();
}


//////////////////* Others *//////////////////
// ��ü�� �ٴ� �߽�, ��, ��, ��, �츦 �������� �Ͽ� ���鿡 �ٵ��� �Ѵ�.
void GameObject::AttachToGround()
{
	Vec3 pos = GetPosition();
	const float terrainHeight = crntScene->GetTerrainHeight(pos.x, pos.z);

	pos.y = terrainHeight;
	SetPosition(pos);
}


void GameObject::TiltToGround()
{
	AttachToGround();

	auto& obbList = GetComponent<ObjectCollider>()->GetOBBList();
	if (obbList.size() > 0) {
		// ����� ���� ���
		// OBB �𼭸� �Ʒ� 4�� �� ���ϱ�
		std::vector<Vec3> corners(8);
		obbList.front()->GetCorners(corners.data());
		const Vec3 leftTop = corners[0];
		const Vec3 rightTop = corners[1];
		const Vec3 leftBottom = corners[4];
		const Vec3 rightBottom = corners[5];

		// weight points
		const Vec3 center = GetPosition();
		const Vec3 front = Vector3::Add(rightTop, Vector3::Subtract(leftTop, rightTop), 0.5f);
		const Vec3 back = Vector3::Add(rightBottom, Vector3::Subtract(leftBottom, rightBottom), 0.5f);
		const Vec3 left = Vector3::Add(center, Vector3::Subtract(leftTop, rightTop), 0.5f);
		const Vec3 right = Vector3::Add(center, Vector3::Subtract(rightTop, leftTop), 0.5f);

		// �� ������ ���� ���� ���� ���
		auto GetHeight = [&](const Vec3& pos) { return pos.y - crntScene->GetTerrainHeight(pos.x, pos.z); };

		float heightFront = GetHeight(front);
		float heightBack = -GetHeight(back);
		float heightLeft = GetHeight(left);
		float heightRight = -GetHeight(right);

		// ��,�� / ��,��� �������� ���� ū ���� ���� ȸ��
		float yaw = max(heightFront, heightBack);
		float roll = max(heightLeft, heightRight);

		// ���鿡 �굵�� ������ ȸ��
		if (!IsZero(yaw) || !IsZero(roll)) {
			Rotate(80.0f * yaw * DeltaTime(), 0.0f, 0.0f);
			Rotate(0.0f, 0.0f, 80.0f * roll * DeltaTime());
		}
	}
}


void GameObject::Enable(bool isUpdateObjectGrid)
{
	mIsActive = true;

	if (isUpdateObjectGrid) {
		crntScene->UpdateObjectGrid(this);
	}
}

void GameObject::Disable(bool isUpdateObjectGrid)
{
	mIsActive = false;

	if (isUpdateObjectGrid) {
		crntScene->RemoveObjectFromGrid(this);
	}
}




// [ InstancinObject ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void InstancinObject::Push()
{
	if (mIsPushed) {
		return;
	}

	mIsPushed = true;
	mBuffer->PushObject(this);
}

void InstancinObject::SetBuffer(rsptr<ObjectInstanceBuffer> buffer)
{
	SetInstancing();
	mBuffer = buffer;

	if (mType == ObjectType::DynamicMove) {
		mUpdate = [this]() { UpdateDynamic(); };
	}
	else {
		mUpdate = [this]() { UpdateStatic(); };
	}
}

void InstancinObject::Render()
{
	Push();
}

void InstancinObject::UpdateStatic()
{
	Reset();
}
void InstancinObject::UpdateDynamic()
{
	GameObject::Update();
	Reset();
}

void InstancinObject::Update()
{
	mUpdate();
}



// [ ObjectInstanceBuffer ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ObjectInstanceBuffer::SetModel(rsptr<const MasterModel> model)
{
	mModel = model;
	Transform::MergeTransform(mMergedTransform, mModel.get());
	mMergedTransform.erase(mMergedTransform.begin());
}

void ObjectInstanceBuffer::CreateShaderVariables(UINT objectCount)
{
	mObjectCount = objectCount;
	::CreateBufferResource(NULL, sizeof(VS_OBJECT_INSTANCE) * mObjectCount, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, mInstBuffer);
	mInstBuffer->Map(0, NULL, (void**)&mMappedBuffer);
}

void ObjectInstanceBuffer::PushObject(InstancinObject* object)
{
	assert(mCrntBufferIndex < mObjectCount);

	XMStoreFloat4x4(&mMappedBuffer[mCrntBufferIndex++].mLocalTransform, XMMatrixTranspose(XMLoadFloat4x4(&object->GetWorldTransform())));
}

void ObjectInstanceBuffer::UpdateShaderVariables() const
{
	cmdList->SetGraphicsRootShaderResourceView(crntScene->GetRootParamIndex(RootParam::Instancing), mInstBuffer->GetGPUVirtualAddress());
}	

void ObjectInstanceBuffer::Render()
{
	if (mModel) {
		mModel->Render(this);
	}

	ResetBuffer();
}