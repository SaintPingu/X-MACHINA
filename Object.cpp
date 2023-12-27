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













// [ CModelObject ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//===== (CModelObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////
CModelObject::CModelObject()
{
	
}

CModelObject::~CModelObject()
{

}

//===== (CModelObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////



//===== (CModelObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Setter ] /////

//////////////////* Position *//////////////////
void CModelObject::SetPosition(const Vec3& pos)
{
	CModelObject::SetPosition(pos.x, pos.y, pos.z);
}

void CModelObject::SetPosition(const XMVECTOR& pos)
{
	Transform::SetPosition(pos);
	//UpdateBoundingBox();
}

void CModelObject::SetPosition(float x, float y, float z)
{
	Transform::SetPosition(x, y, z);
	auto& collider = GetComponent<ObjectCollider>();
	if (collider) {
		collider->Update();
	}
}


//////////////////* Others *//////////////////


//===== (CModelObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

//////////////////* DirectX *//////////////////
void CModelObject::CreateShaderVariables()
{

}

void CModelObject::UpdateShaderVariables() const
{
	crntScene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrixTranspose(XMLoadFloat4x4(&GetWorldTransform())), 0);
}


void CModelObject::ReleaseShaderVariables()
{

}

void CModelObject::ReleaseUploadBuffers()
{
	CObject::ReleaseUploadBuffers();
}




//////////////////* Movement *//////////////////
void CModelObject::Translate(const Vec3& translation)
{
	Transform::Translate(translation);
}

void CModelObject::Translate(const Vec3& direction, float distance)
{
	CModelObject::Translate(Vector3::ScalarProduct(direction, distance));
}

void CModelObject::Translate(float x, float y, float z)
{
	CModelObject::Translate(Vec3(x, y, z));
}

void CModelObject::Rotate(float pitch, float yaw, float roll)
{
	Transform::Rotate(pitch, yaw, roll);
}

void CModelObject::Rotate(const Vec3& axis, float angle)
{
	Transform::Rotate(axis, angle);
}


//////////////////* Transform *//////////////////
void CModelObject::SetWorldTransform(const Vec4x4& transform)
{
	Transform::SetWorldTransform(transform);
}

void CModelObject::SetTransform(const Vec4x4& transform)
{
	Transform::SetTransform(transform);
}


//////////////////* Others *//////////////////
void CModelObject::Update()
{
	CObject::Update();
}




void CModelObject::ToggleDrawBoundings()
{
	mIsDrawBounding = !mIsDrawBounding;

	if (mSibling) {
		mSibling->Object<CGameObject>()->ToggleDrawBoundings();
	}
	if (mChild) {
		mChild->Object<CGameObject>()->ToggleDrawBoundings();
	}
}

Transform* CModelObject::FindFrame(const std::string& frameName)
{
	if (mName == frameName) {
		return this;
	}

	Transform* transform{};
	if (mSibling) {
		if (transform = mSibling->Object<CModelObject>()->FindFrame(frameName)) {
			return transform;
		}
	}
	if (mChild) {
		if (transform = mChild->Object<CModelObject>()->FindFrame(frameName)) {
			return transform;
		}
	}

	return nullptr;
}

CModelObject* CModelObject::FindObject(const std::string& frameName)
{
	if (mName == frameName) {
		return this;
	}

	if (mSibling) {
		CModelObject* object = mSibling->Object<CModelObject>()->FindObject(frameName);
		if (object) {
			return object;
		}
	}
	if (mChild) {
		CModelObject* object = mChild->Object<CModelObject>()->FindObject(frameName);
		if (object) {
			return object;
		}
	}

	return nullptr;
}



//////////////////* Lights *//////////////////
//void CModelObject::SetLightRange(LIGHT_RANGE lightRange)
//{
//	mLightRange = lightRange;
//}
//
//void CModelObject::SetAllLight(const LIGHT* light)
//{
//	assert(light);
//	assert(mLightRange.lights);
//
//	for (size_t i = mLightRange.begin; i < mLightRange.end; ++i) {
//		mLightRange.Get(i) = *light;
//		mLightRange.Get(i).mIsEnable = true;
//	}
//}



// [ CGameObject ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//===== (CGameObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////

CGameObject::CGameObject() : CModelObject()
{
	AddComponent<ObjectCollider>();
}

CGameObject::~CGameObject()
{

}



//===== (CGameObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////

bool CGameObject::IsTransparent() const
{
	return mLayer == ObjectLayer::Transparent;
}

rsptr<CTexture> CGameObject::GetTexture() const
{
	return mModel->GetTexture();
}

//===== (CGameObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Setter ] /////

void CGameObject::SetModel(rsptr<const CMasterModel> model)
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

void CGameObject::SetFlyable(bool isFlyable)
{
	mIsFlyable = isFlyable;
}

//===== (CGameObject) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

//////////////////* DirectX *//////////////////
void CGameObject::UpdateShaderVariablesSprite()
{

}

void CGameObject::Render()
{
	if (mModel) {
		mModel->Render(this);
	}
}


void CGameObject::RenderBounds()
{
	if (mIsDrawBounding) {
		GetComponent<ObjectCollider>()->Render();
	}
}



//////////////////* Movement *//////////////////
void CGameObject::Update()
{
	if (!IsActive()) {
		return;
	}

	if (!mIsFlyable) {
		TiltToGround();
	}

	CModelObject::Update();
}


//////////////////* Others *//////////////////
// 객체의 바닥 중심, 앞, 뒤, 좌, 우를 기준으로 하여 지면에 붙도록 한다.
void CGameObject::AttachToGround()
{
	Vec3 pos = GetPosition();
	const float terrainHeight = crntScene->GetTerrainHeight(pos.x, pos.z);

	pos.y = terrainHeight;
	SetPosition(pos);
}


void CGameObject::TiltToGround()
{
	AttachToGround();

	auto& obbList = GetComponent<ObjectCollider>()->GetOBBList();
	if (obbList.size() > 0) {
		// 기울임 각도 계산
		// OBB 모서리 아래 4개 점 구하기
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

		// 각 지점에 대해 높이 차이 계산
		auto GetHeight = [&](const Vec3& pos) { return pos.y - crntScene->GetTerrainHeight(pos.x, pos.z); };

		float heightFront = GetHeight(front);
		float heightBack = -GetHeight(back);
		float heightLeft = GetHeight(left);
		float heightRight = -GetHeight(right);

		// 앞,뒤 / 좌,우로 높이차가 가장 큰 값에 대해 회전
		float yaw = max(heightFront, heightBack);
		float roll = max(heightLeft, heightRight);

		// 지면에 닿도록 각도로 회전
		if (!IsZero(yaw) || !IsZero(roll)) {
			Rotate(80.0f * yaw * DeltaTime(), 0.0f, 0.0f);
			Rotate(0.0f, 0.0f, 80.0f * roll * DeltaTime());
		}
	}
}


void CGameObject::Enable(bool isUpdateObjectGrid)
{
	mIsActive = true;

	if (isUpdateObjectGrid) {
		crntScene->UpdateObjectGrid(this);
	}
}

void CGameObject::Disable(bool isUpdateObjectGrid)
{
	mIsActive = false;

	if (isUpdateObjectGrid) {
		crntScene->RemoveObjectFromGrid(this);
	}
}




// [ CInstancingObject ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CInstancingObject::Push()
{
	if (mIsPushed) {
		return;
	}

	mIsPushed = true;
	mBuffer->PushObject(this);
}

void CInstancingObject::SetBuffer(rsptr<CObjectInstanceBuffer> buffer)
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

void CInstancingObject::Render()
{
	Push();
}

void CInstancingObject::UpdateStatic()
{
	Reset();
}
void CInstancingObject::UpdateDynamic()
{
	CGameObject::Update();
	Reset();
}

void CInstancingObject::Update()
{
	mUpdate();
}



// [ CObjectInstanceBuffer ] //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CObjectInstanceBuffer::SetModel(rsptr<const CMasterModel> model)
{
	mModel = model;
	Transform::MergeTransform(mMergedTransform, mModel.get());
	mMergedTransform.erase(mMergedTransform.begin());
}

void CObjectInstanceBuffer::CreateShaderVariables(UINT objectCount)
{
	mObjectCount = objectCount;
	::CreateBufferResource(NULL, sizeof(VS_OBJECT_INSTANCE) * mObjectCount, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, mInstBuffer);
	mInstBuffer->Map(0, NULL, (void**)&mMappedBuffer);
}

void CObjectInstanceBuffer::PushObject(CInstancingObject* object)
{
	assert(mCrntBufferIndex < mObjectCount);

	XMStoreFloat4x4(&mMappedBuffer[mCrntBufferIndex++].mLocalTransform, XMMatrixTranspose(XMLoadFloat4x4(&object->GetWorldTransform())));
}

void CObjectInstanceBuffer::UpdateShaderVariables() const
{
	cmdList->SetGraphicsRootShaderResourceView(crntScene->GetRootParamIndex(RootParam::Instancing), mInstBuffer->GetGPUVirtualAddress());
}	

void CObjectInstanceBuffer::Render()
{
	if (mModel) {
		mModel->Render(this);
	}

	ResetBuffer();
}