#include "stdafx.h"
#include "Transform.h"
#include "DXGIMgr.h"

#include "Scene.h"











//===== (Transform) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Constructor ] /////

Transform::Transform()
{
	
}

//===== (Transform) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Getter ] /////

UINT Transform::GetTransformCount() const
{
	UINT result{};
	Transform::GetTransformCount(result, this);
	return result;
}

//===== (Transform) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Setter ] /////

///////////////////////////* Position *///////////////////////////
void Transform::SetPosition(float x, float y, float z)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.z = z;

	UpdateTransform();
	ComputeWorldTransform();
}

void Transform::SetPosition(const Vec3& pos)
{
	Transform::SetPosition(pos.x, pos.y, pos.z);
}

void Transform::SetPosition(const XMVECTOR& pos)
{
	XMStoreFloat3(&mPosition, pos);

	UpdateTransform();
	ComputeWorldTransform();
}

void Transform::SetPositionX(float x)
{
	Transform::SetPosition(x, mPosition.y, mPosition.z);
}

void Transform::SetPositionY(float y)
{
	Transform::SetPosition(mPosition.x, y, mPosition.z);
}

void Transform::SetPositionZ(float z)
{
	Transform::SetPosition(mPosition.x, mPosition.y, z);
}


///////////////////////////* Axis *///////////////////////////
void Transform::SetAxis(const Vec3& look, const Vec3& up, const Vec3& right)
{
	mLook = look;
	mUp = up;
	mRight = right;

	UpdateTransform();
	ComputeWorldTransform();
}

void Transform::SetRight(const Vec3& right)
{
	mRight = Vector3::Normalize(right);
	mUp = Vector3::Normalize(Vector3::CrossProduct(mLook, mRight));
	mLook = Vector3::Normalize(Vector3::CrossProduct(mRight, mUp));

	UpdateTransform();
	ComputeWorldTransform();
}

void Transform::SetLook(const Vec3& look)
{
	mLook = Vector3::Normalize(look);
	mUp = Vector3::Normalize(Vector3::CrossProduct(mLook, mRight));
	mRight = Vector3::Normalize(Vector3::CrossProduct(mLook, mUp));

	UpdateTransform();
	ComputeWorldTransform();
}

void Transform::SetUp(const Vec3& up)
{
	mUp = Vector3::Normalize(up);
	mRight = Vector3::Normalize(Vector3::CrossProduct(mLook, mUp));
	mLook = Vector3::Normalize(Vector3::CrossProduct(mRight, mUp));

	UpdateTransform();
	ComputeWorldTransform();
}


///////////////////////////* Others *///////////////////////////
void Transform::SetChild(rsptr<Transform> child)
{
	if (mChild) {
		if (mChild->mSibling) {
			Transform* tail = mChild->mSibling.get();
			while (tail->mSibling) {
				tail = tail->mSibling.get();
			}
			tail->mSibling = child;
		}
		else {
			if (child) {
				child->mSibling = mChild->mSibling;
			}
			mChild->mSibling = child;
		}
	}
	else {
		mChild = child;;
	}
	if (child) child->mParent = this;
}

void Transform::SetTransform(const Vec4x4& transform)
{
	mLocalTransform = transform;
	mPrevTransform = transform;
	UpdateAxis();
}




//===== (Transform) =====//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// [ Others ] /////

///////////////////////////* Translate *///////////////////////////
void Transform::Translate(const Vec3& translation)
{
	if (Vector3::Length(translation) <= FLT_EPSILON) {
		return;
	}

	mPosition = Vector3::Add(mPosition, translation);

	UpdateTransform();
	ComputeWorldTransform();
}

void Transform::Translate(const Vec3& direction, float distance)
{
	Transform::Translate(Vector3::ScalarProduct(direction, distance));
}

void Transform::Translate(float x, float y, float z)
{
	Transform::Translate(Vec3(x, y, z));
}


///////////////////////////* Movement *///////////////////////////
void Transform::MoveLocal(const Vec3& translation)
{
	Vec3 right = Vector3::ScalarProduct(mRight, translation.x);
	Vec3 up = Vector3::ScalarProduct(mUp, translation.y);
	Vec3 left = Vector3::ScalarProduct(mLook, translation.z);

	Transform::Translate(Vector3::Add(right, up, left));
}

void Transform::MoveStrafe(float fDistance)
{
	Transform::Translate(mRight, fDistance);
}

void Transform::MoveUp(float fDistance)
{
	Transform::Translate(mUp, fDistance);
}

void Transform::MoveForward(float fDistance)
{
	Transform::Translate(mLook, fDistance);
}


///////////////////////////* Rotation *///////////////////////////
void Transform::Rotate(float pitch, float yaw, float roll)
{
	if (pitch != 0.0f) {
		mPitch += pitch;
	}
	if (yaw != 0.0f) {
		mYaw += yaw;
		if (mYaw > 360.0f) mYaw -= 360.0f;
		if (mYaw < 0.0f) mYaw += 360.0f;
	}
	if (roll != 0.0f) {
		mRoll += roll;
	}

	UpdateTransform();

	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
	mLocalTransform = Matrix4x4::Multiply(mtxRotate, mLocalTransform);

	UpdateAxis();
	ComputeWorldTransform();
}

void Transform::Rotate(const Vec3& axis, float angle)
{
	mLocalTransform = Matrix4x4::Multiply(XMMatrix::RotationAxis(axis, angle), mLocalTransform);
	SetTransform(mLocalTransform);
}

void Transform::RotateOffset(const Vec3& axis, float angle, const Vec3& xmf3Offset)
{
	UpdateTransform();

	Vec4x4 mtxTranslateToOrigin = Matrix4x4::Translate(-xmf3Offset.x, -xmf3Offset.y, -xmf3Offset.z);
	Vec4x4 mtxRotate = Matrix4x4::RotationAxis(axis, angle);
	Vec4x4 mtxTranslateBack = Matrix4x4::Translate(xmf3Offset.x, xmf3Offset.y, xmf3Offset.z);

	mLocalTransform = Matrix4x4::Multiply(mtxTranslateToOrigin, mLocalTransform);
	mLocalTransform = Matrix4x4::Multiply(mtxRotate, mLocalTransform);
	mLocalTransform = Matrix4x4::Multiply(mtxTranslateBack, mLocalTransform);

	UpdateAxis();
	ComputeWorldTransform();
}


///////////////////////////* Look *///////////////////////////
void Transform::LookTo(const Vec3& xmf3LookTo, const Vec3& up)
{
	Vec4x4 xmf4x4View = Matrix4x4::LookToLH(GetPosition(), xmf3LookTo, up);
	mLocalTransform._11 = xmf4x4View._11; mLocalTransform._12 = xmf4x4View._21; mLocalTransform._13 = xmf4x4View._31;
	mLocalTransform._21 = xmf4x4View._12; mLocalTransform._22 = xmf4x4View._22; mLocalTransform._23 = xmf4x4View._32;
	mLocalTransform._31 = xmf4x4View._13; mLocalTransform._32 = xmf4x4View._23; mLocalTransform._33 = xmf4x4View._33;

	UpdateAxis();
	ComputeWorldTransform();
}

void Transform::LookAt(const Vec3& lookAt, const Vec3& up)
{
	Vec4x4 xmf4x4View = Matrix4x4::LookAtLH(GetPosition(), lookAt, up);
	mLocalTransform._11 = xmf4x4View._11; mLocalTransform._12 = xmf4x4View._21; mLocalTransform._13 = xmf4x4View._31;
	mLocalTransform._21 = xmf4x4View._12; mLocalTransform._22 = xmf4x4View._22; mLocalTransform._23 = xmf4x4View._32;
	mLocalTransform._31 = xmf4x4View._13; mLocalTransform._32 = xmf4x4View._23; mLocalTransform._33 = xmf4x4View._33;

	UpdateAxis();
	ComputeWorldTransform();
}


///////////////////////////* Others *///////////////////////////
void Transform::UpdateTransform()
{
	if (isUpdated) {
		XMStoreFloat4x4(&mPrevTransform, _MATRIX(mLocalTransform));
		isUpdated = false;
	}

	mLocalTransform._11 = mRight.x;
	mLocalTransform._12 = mRight.y;
	mLocalTransform._13 = mRight.z;

	mLocalTransform._21 = mUp.x;
	mLocalTransform._22 = mUp.y;
	mLocalTransform._23 = mUp.z;

	mLocalTransform._31 = mLook.x;
	mLocalTransform._32 = mLook.y;
	mLocalTransform._33 = mLook.z;

	mLocalTransform._41 = mPosition.x;
	mLocalTransform._42 = mPosition.y;
	mLocalTransform._43 = mPosition.z;
}

void Transform::UpdateAxis()
{
	mRight.x = mLocalTransform._11;
	mRight.y = mLocalTransform._12;
	mRight.z = mLocalTransform._13;

	mUp.x = mLocalTransform._21;
	mUp.y = mLocalTransform._22;
	mUp.z = mLocalTransform._23;

	mLook.x = mLocalTransform._31;
	mLook.y = mLocalTransform._32;
	mLook.z = mLocalTransform._33;

	mPosition.x = mLocalTransform._41;
	mPosition.y = mLocalTransform._42;
	mPosition.z = mLocalTransform._43;
}

void Transform::RequestTransform(Vec4x4& parentTransform)
{
	if (mParent) {
		mParent->RequestTransform(parentTransform);
		parentTransform = Matrix4x4::Multiply(mLocalTransform, parentTransform);
		return;
	}

	parentTransform = mWorldTransform;
}

void Transform::Init()
{
	ComputeWorldTransform();
}

void Transform::Update()
{
	isUpdated = true;
	ComputeWorldTransform();
}

void Transform::ComputeWorldTransform(const Vec4x4* parentTransform)
{
	// must be start from parent
	if (!parentTransform && mParent) {
		Vec4x4 transform;
		mParent->RequestTransform(transform);
		mWorldTransform = Matrix4x4::Multiply(mLocalTransform, transform);

		if (mSibling) {
			mSibling->ComputeWorldTransform(&transform);
		}
	}
	else {
		mWorldTransform = (parentTransform) ? Matrix4x4::Multiply(mLocalTransform, *parentTransform) : mLocalTransform;

		if (mSibling) {
			mSibling->ComputeWorldTransform(parentTransform);
		}
	}

	if (mChild) {
		mChild->ComputeWorldTransform(&mWorldTransform);
	}
}

void Transform::UpdateShaderVariableTransform() const
{
	XMMATRIX mtx = _MATRIX(GetWorldTransform());
	if (crntScene->IsRenderReflectObject()) {
		XMMATRIX mtxReflect = crntScene->GetReflect();
		mtx = XMMatrixMultiply(mtx, mtxReflect);
	}
	crntScene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrixTranspose(mtx), 0);
}

void Transform::SetWorldTransform(const Vec4x4& transform)
{
	mWorldTransform = transform;
	mLocalTransform = transform;
	mPrevTransform = transform;
	UpdateAxis();
	ComputeWorldTransform();
}

void Transform::ReturnTransform()
{
	if (!mParent) {
		XMStoreFloat4x4(&mLocalTransform, _MATRIX(mPrevTransform));
		UpdateAxis();
		ComputeWorldTransform();
	}
}

void Transform::NormalizeAxis()
{
	mLook = Vector3::Normalize(mLook);
	mRight = Vector3::CrossProduct(mUp, mLook, true);
	mUp = Vector3::CrossProduct(mLook, mRight, true);

	UpdateTransform();
}

Vec3 Transform::GetDirection(DWORD dwDirection, float distance) const
{
	if (!dwDirection) {
		return Vec3{};
	}

	Vec3 direction{};

	if (dwDirection & DIR_FORWARD)	direction = Vector3::Add(direction, mLook, 1.0f);
	if (dwDirection & DIR_BACKWARD) direction = Vector3::Add(direction, mLook, -1.0f);
	if (dwDirection & DIR_RIGHT)	direction = Vector3::Add(direction, mRight, 1.0f);
	if (dwDirection & DIR_LEFT)		direction = Vector3::Add(direction, mRight, -1.0f);

	if (dwDirection & DIR_UP)		direction = Vector3::Add(direction, mUp, 1.0f);
	if (dwDirection & DIR_DOWN)		direction = Vector3::Add(direction, mUp, -1.0f);

	return direction;
}









void Transform::MergeTransform(std::vector<const Transform*>& out, const Transform* transform)
{
	out.emplace_back(transform);

	if (transform->mSibling) {
		MergeTransform(out, transform->mSibling.get());
	}
	if (transform->mChild) {
		MergeTransform(out, transform->mChild.get());
	}
}

void Transform::GetTransformCount(UINT& out, const Transform* transform)
{
	++out;

	if (transform->mSibling) {
		GetTransformCount(out, transform->mSibling.get());
	}
	if (transform->mChild) {
		GetTransformCount(out, transform->mChild.get());
	}
}

void Transform::UpdateShaderVariables(const XMMATRIX& matrix)
{
	crntScene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrixTranspose(matrix), 0);
}