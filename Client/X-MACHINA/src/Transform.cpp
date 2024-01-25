#include "stdafx.h"
#include "Transform.h"
#include "DXGIMgr.h"
#include "FrameResource.h"
#include "Scene.h"


#pragma region Getter
Vec3 Transform::GetDirection(DWORD dwDirection, float distance) const
{
	if (!dwDirection) {
		return Vector3::Zero();
	}

	Vec3 result{};

	if (dwDirection & Dir::Front)	result = Vector3::Add(result, mLook, 1.f);
	if (dwDirection & Dir::Back)	result = Vector3::Add(result, mLook, -1.f);
	if (dwDirection & Dir::Right)	result = Vector3::Add(result, mRight, 1.f);
	if (dwDirection & Dir::Left)	result = Vector3::Add(result, mRight, -1.f);

	if (dwDirection & Dir::Up)		result = Vector3::Add(result, mUp, 1.f);
	if (dwDirection & Dir::Down)	result = Vector3::Add(result, mUp, -1.f);

	return result;
}

#pragma endregion

#pragma region Setter
void Transform::SetPosition(float x, float y, float z)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.z = z;

	UpdateLocalTransform();
}

void Transform::SetPosition(const Vec3& pos)
{
	SetPosition(pos.x, pos.y, pos.z);
}

void Transform::SetPosition(const Vector& pos)
{
	XMStoreFloat3(&mPosition, pos);

	UpdateLocalTransform();
}

void Transform::SetPositionX(float x)
{
	SetPosition(x, mPosition.y, mPosition.z);
}

void Transform::SetPositionY(float y)
{
	SetPosition(mPosition.x, y, mPosition.z);
}

void Transform::SetPositionZ(float z)
{
	SetPosition(mPosition.x, mPosition.y, z);
}

void Transform::SetAxis(const Vec3& look, const Vec3& up, const Vec3& right)
{
	mLook = look;
	mUp = up;
	mRight = right;

	UpdateLocalTransform();
}

void Transform::SetAxis(const Vec4x4& axisMatrix)
{
	::memcpy(&mLocalTransform, &axisMatrix, sizeof(Vec4x3));

	UpdateAxis();
}

void Transform::SetRight(const Vec3& right)
{
	mRight = Vector3::Normalize(right);
	mUp = Vector3::Normalize(Vector3::CrossProduct(mLook, mRight));
	mLook = Vector3::Normalize(Vector3::CrossProduct(mRight, mUp));

	UpdateLocalTransform();
}

void Transform::SetLook(const Vec3& look)
{
	mLook = Vector3::Normalize(look);
	mUp = Vector3::Normalize(Vector3::CrossProduct(mLook, mRight));
	mRight = Vector3::Normalize(Vector3::CrossProduct(mLook, mUp));

	UpdateLocalTransform();
}

void Transform::SetUp(const Vec3& up)
{
	mUp = Vector3::Normalize(up);
	mRight = Vector3::Normalize(Vector3::CrossProduct(mLook, mUp));
	mLook = Vector3::Normalize(Vector3::CrossProduct(mRight, mUp));

	UpdateLocalTransform();
}

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

void Transform::SetLocalTransform(const Vec4x4& transform)
{
	mLocalTransform = transform;
	mPrevTransform = transform;
	UpdateAxis(false);
}
#pragma endregion




///////////////////////////* Translate *///////////////////////////
void Transform::Translate(const Vec3& translation)
{
	if (Vector3::Length(translation) <= FLT_EPSILON) {
		return;
	}

	mPosition = Vector3::Add(mPosition, translation);

	UpdateLocalTransform();
}

void Transform::Translate(const Vec3& direction, float distance)
{
	Translate(Vector3::ScalarProduct(direction, distance));
}

void Transform::Translate(float x, float y, float z)
{
	Translate(Vec3(x, y, z));
}


///////////////////////////* Movement *///////////////////////////
void Transform::MoveLocal(const Vec3& translation)
{
	Vec3 right = Vector3::ScalarProduct(mRight, translation.x);
	Vec3 up = Vector3::ScalarProduct(mUp, translation.y);
	Vec3 left = Vector3::ScalarProduct(mLook, translation.z);

	Translate(Vector3::Add(right, up, left));
}

void Transform::MoveStrafe(float distance)
{
	Translate(mRight, distance);
}

void Transform::MoveUp(float distance)
{
	Translate(mUp, distance);
}

void Transform::MoveForward(float distance)
{
	Translate(mLook, distance);
}


///////////////////////////* Rotation *///////////////////////////
void Transform::Rotate(float pitch, float yaw, float roll)
{
	if (pitch != 0.f) {
		mPitch += pitch;
	}
	if (yaw != 0.f) {
		mYaw += yaw;
		if (mYaw > 360.f) {
			mYaw -= 360.f;
		}
		if (mYaw < 0.f) {
			mYaw += 360.f;
		}
	}
	if (roll != 0.f) {
		mRoll += roll;
	}

	Matrix mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
	mLocalTransform = Matrix4x4::Multiply(mtxRotate, mLocalTransform);

	UpdateAxis();
}

void Transform::Rotate(const Vec3& axis, float angle)
{
	mLocalTransform = Matrix4x4::Multiply(XMMatrix::RotationAxis(axis, angle), mLocalTransform);
	SetLocalTransform(mLocalTransform);
}

void Transform::RotateOffset(const Vec3& axis, float angle, const Vec3& offset)
{
	Vec4x4 mtxTranslateToOrigin = Matrix4x4::Translate(-offset.x, -offset.y, -offset.z);
	Vec4x4 mtxRotate = Matrix4x4::RotationAxis(axis, angle);
	Vec4x4 mtxTranslateBack = Matrix4x4::Translate(offset.x, offset.y, offset.z);

	mLocalTransform = Matrix4x4::Multiply(mtxTranslateToOrigin, mLocalTransform);
	mLocalTransform = Matrix4x4::Multiply(mtxRotate, mLocalTransform);
	mLocalTransform = Matrix4x4::Multiply(mtxTranslateBack, mLocalTransform);

	UpdateAxis();
}

void Transform::LookTo(const Vec3& lookTo, const Vec3& up)
{
	SetAxis(Matrix4x4::LookToLH(GetPosition(), lookTo, up, true));
}

void Transform::LookAt(const Vec3& lookAt, const Vec3& up)
{
	SetAxis(Matrix4x4::LookAtLH(GetPosition(), lookAt, up, true));
}


///////////////////////////* Transform *///////////////////////////
void Transform::SetWorldTransform(const Vec4x4& transform)
{
	::memcpy(&mWorldTransform, &transform, sizeof(Vec4x4));
	::memcpy(&mLocalTransform, &transform, sizeof(Vec4x4));
	::memcpy(&mPrevTransform, &transform, sizeof(Vec4x4));

	UpdateAxis();
}

void Transform::ReturnToPrevTransform()
{
	if (!mParent) {
		XMStoreFloat4x4(&mLocalTransform, _MATRIX(mPrevTransform));

		UpdateAxis();
	}
}

void Transform::UpdateAxis(bool isComputeWorldTransform)
{
	::memcpy(&mRight, &mLocalTransform._11, sizeof(Vec3));
	::memcpy(&mUp, &mLocalTransform._21, sizeof(Vec3));
	::memcpy(&mLook, &mLocalTransform._31, sizeof(Vec3));
	::memcpy(&mPosition, &mLocalTransform._41, sizeof(Vec3));

	if (isComputeWorldTransform) {
		ComputeWorldTransform();
	}
}

void Transform::UpdateLocalTransform(bool isComputeWorldTransform)
{
	::memcpy(&mLocalTransform._11, &mRight, sizeof(Vec3));
	::memcpy(&mLocalTransform._21, &mUp, sizeof(Vec3));
	::memcpy(&mLocalTransform._31, &mLook, sizeof(Vec3));
	::memcpy(&mLocalTransform._41, &mPosition, sizeof(Vec3));

	if (isComputeWorldTransform) {
		ComputeWorldTransform();
	}
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

void Transform::Awake()
{
	ComputeWorldTransform();
}

void Transform::OnDestroy()
{
	// 객체 파괴시 사용한 모든 오브젝트 상수 버퍼 인덱스를 반환한다.
	if (mUseObjCB) {
		ReturnObjCBIndex();
	}
}

void Transform::ReturnObjCBIndex()
{
	// 실제 사용 횟수만큼만 인덱스를 반환한다.
	mObjCBIndices.resize(mObjCBCount);
	for (const int index : mObjCBIndices) {
		frmResMgr->ReturnIndex(index, BufferType::Object);
	}

	if (mSibling) {
		mSibling->ReturnObjCBIndex();
	}
	if (mChild) {
		mChild->ReturnObjCBIndex();
	}
}

void Transform::BeforeUpdateTransform()
{
	XMStoreFloat4x4(&mPrevTransform, _MATRIX(mLocalTransform));
}

void Transform::UpdateShaderVars(const int cnt, const int matIndex) const
{
	// 실제 사용 횟수를 저장한다.
	if (mObjCBCount <= cnt) {
		mObjCBCount = cnt + 1;
	}

	ObjectConstants objectConstants;
	objectConstants.MtxWorld = XMMatrixTranspose(_MATRIX(GetWorldTransform()));
	objectConstants.MatIndex = matIndex;
	
	frmResMgr->CopyData(mObjCBIndices[cnt], objectConstants);

	scene->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(mObjCBIndices[cnt]));
}

void Transform::NormalizeAxis()
{
	mLook = Vector3::Normalize(mLook);
	mRight = Vector3::CrossProduct(mUp, mLook, true);
	mUp = Vector3::CrossProduct(mLook, mRight, true);

	UpdateLocalTransform(false);
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

void Transform::UpdateShaderVars(const Matrix& matrix)
{
	scene->SetGraphicsRoot32BitConstants(RootParam::Collider, XMMatrixTranspose(matrix), 0);
}