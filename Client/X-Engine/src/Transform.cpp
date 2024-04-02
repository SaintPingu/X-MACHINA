#include "EnginePch.h"
#include "Component/Transform.h"
#include "DXGIMgr.h"
#include "FrameResource.h"

#pragma region Getter
Vec4 Transform::GetLocalRotation() const
{
	return Quat::CreateFromRotationMatrix(mLocalTransform);
}

Vec4 Transform::GetRotation() const
{
	return Quat::CreateFromRotationMatrix(mWorldTransform);
}

Vec3 Transform::GetDirection(DWORD dwDirection, float distance) const
{
	if (!dwDirection) {
		return Vector3::Zero();
	}

	Vec3 result{};

	if (dwDirection & Dir::Front)	result += mLook * 1.f;
	if (dwDirection & Dir::Back)	result += mLook * -1.f;
	if (dwDirection & Dir::Right)	result += mRight * 1.f;
	if (dwDirection & Dir::Left)	result += mRight * -1.f;

	if (dwDirection & Dir::Up)		result += mUp * 1.f;
	if (dwDirection & Dir::Down)	result += mUp * -1.f;

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

void Transform::SetAxis(const Matrix& axisMatrix)
{
	::memcpy(&mLocalTransform, &axisMatrix, sizeof(XMFLOAT4X3));

	UpdateAxis();
}

void Transform::SetRight(const Vec3& right)
{
	mRight = Vector3::Normalized(right);
	mUp    = Vector3::Normalized(mLook.Cross(mRight));
	mLook  = Vector3::Normalized(mRight.Cross(mUp));

	UpdateLocalTransform();
}

void Transform::SetLook(const Vec3& look)
{
	mLook  = Vector3::Normalized(look);
	mUp    = Vector3::Normalized(mLook.Cross(mRight));
	mRight = Vector3::Normalized(mLook.Cross(mUp));

	UpdateLocalTransform();
}

void Transform::SetUp(const Vec3& up)
{
	mUp    = Vector3::Normalized(up);
	mRight = Vector3::Normalized(mLook.Cross(mUp));
	mLook  = Vector3::Normalized(mRight.Cross(mUp));

	UpdateLocalTransform();
}

void Transform::SetChild(rsptr<Transform> child)
{
	if (child) {
		child->mParent = this;
	}

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
		mChild = child;
	}
}

void Transform::SetLocalTransform(const Matrix& transform)
{
	mLocalTransform = transform;
	mPrevTransform = transform;
	UpdateAxis();
}
#pragma endregion




///////////////////////////* Translate *///////////////////////////
void Transform::Translate(const Vec3& translation)
{
	if (translation.Length() <= FLT_EPSILON) {
		return;
	}

	mPosition += translation;

	UpdateLocalTransform();
}

void Transform::Translate(const Vec3& direction, float distance)
{
	Translate(direction * distance);
}

void Transform::Translate(float x, float y, float z)
{
	Translate(Vec3(x, y, z));
}


///////////////////////////* Movement *///////////////////////////
void Transform::MoveLocal(const Vec3& translation)
{
	Vec3 right = mRight * translation.x;
	Vec3 up = mUp * translation.y;
	Vec3 left = mLook * translation.z;

	Translate(right + up + left);
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
	Matrix mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll));
	mLocalTransform = mtxRotate * mLocalTransform;

	UpdateAxis();
}

void Transform::Rotate(const Vec3& axis, float angle)
{
	mLocalTransform = Matrix::CreateFromAxisAngle(axis, XMConvertToRadians(angle)) * mLocalTransform;
	SetLocalTransform(mLocalTransform);
}

void Transform::Rotate(const Vec4 quaternion)
{
	mLocalTransform = Matrix::CreateFromQuaternion(quaternion) * mLocalTransform;
	SetLocalTransform(mLocalTransform);
}

void Transform::RotateGlobal(const Vec3& axis, float angle)
{
	//// <Algorithm>
	//// 1. 기존 회전값과 월드 회전값을 미리 저장한다.
	//// 2. 월드 회전값의 역으로 회전하여 좌표축을 정상(왼손 좌표계)으로 되돌린다.
	//// 3. 해당 좌표축은 월드 공간이다. 원하는 축으로 회전한다.
	//// 4. 1에서 저장한 월드 회전값과 기존 회전값을 적용한다.

	XMVECTOR originLocalRotation = _VECTOR4(GetLocalRotation());
	SetLocalRotation(Quat::Identity);
	XMVECTOR worldRotation = _VECTOR4(GetRotation());
	XMVECTOR localRotation = XMQuaternionInverse(worldRotation);

	XMVECTOR quatAngle = XMQuaternionRotationAxis(_VECTOR(axis), XMConvertToRadians(angle));

	localRotation = XMQuaternionMultiply(quatAngle, localRotation);
	localRotation = XMQuaternionMultiply(worldRotation, localRotation);
	localRotation = XMQuaternionMultiply(originLocalRotation, localRotation);

	Quat result;
	XMStoreFloat4(&result, localRotation);
	SetLocalRotation(result);
}

void Transform::RotateGlobal(const Vec3& eulerAngles)
{
	XMVECTOR originLocalRotation = _VECTOR4(GetLocalRotation());
	SetLocalRotation(Quat::Identity);
	XMVECTOR worldRotation = _VECTOR4(GetRotation());
	XMVECTOR localRotation = XMQuaternionInverse(worldRotation);

	XMVECTOR quatAngle = _VECTOR4(Quaternion::ToQuaternion(eulerAngles));

	localRotation = XMQuaternionMultiply(quatAngle, localRotation);
	localRotation = XMQuaternionMultiply(worldRotation, localRotation);
	localRotation = XMQuaternionMultiply(originLocalRotation, localRotation);

	Quat result;
	XMStoreFloat4(&result, localRotation);
	SetLocalRotation(result);
}

void Transform::RotateOffset(const Vec3& axis, float angle, const Vec3& offset)
{
	Matrix mtxTranslateToOrigin = Matrix::CreateTranslation(-offset.x, -offset.y, -offset.z);
	Matrix mtxRotate = Matrix::CreateFromAxisAngle(axis, XMConvertToRadians(angle));
	Matrix mtxTranslateBack = Matrix::CreateTranslation(offset.x, offset.y, offset.z);

	mLocalTransform = mtxTranslateToOrigin * mLocalTransform;
	mLocalTransform = mtxRotate * mLocalTransform;
	mLocalTransform = mtxTranslateBack * mLocalTransform;

	UpdateAxis();
}

bool Transform::RotateTargetAxisY(const Vec3& target, float rotationSpeed)
{
	float dot = mRight.Dot(target - GetPosition());

	if (fabs(dot) < 0.1f) {
		return false;
	}

	int dir = 1;
	if (dot < 0) {
		dir = -1;
	}
	
	Rotate(Vec3::Up, dir * rotationSpeed);
	return true;
}

void Transform::SetLocalRotation(const Vec4& quaternion)
{
	Matrix4x4::SetRotation(mLocalTransform, quaternion);

	UpdateAxis();
}

void Transform::LookTo(const Vec3& lookTo, const Vec3& up)
{
	SetAxis(Matrix4x4::LookToLH(GetPosition(), lookTo, up, false));
}

void Transform::LookAt(const Vec3& lookAt, const Vec3& up)
{
	SetAxis(Matrix4x4::LookAtLH(GetPosition(), lookAt, up, false));
}

void Transform::LookToWorld(const Vec3& lookTo, const Vec3& up)
{
	LookToWorld2(lookTo, GetLook(), up);
}

void Transform::LookToWorld2(const Vec3& lookTo, const Vec3& look, const Vec3& up)
{
	Vec3 from = look.xz();
	Vec3 to   = lookTo.xz();

	float angle = Vector3::SignedAngle(from, to, up);

	RotateGlobal(up, angle);
}


// <보류-민동현>
//Quat originLocalRotation = GetLocalRotation();
//SetLocalRotation(Quat::Identity);
//Quat worldRotation = GetRotation();
//Quat localRotation = Quaternion::Inverse(worldRotation);
//
//Quat angle1 = Quaternion::LookRotation(lookTo, up);
//Quat angle2 = Quaternion::LookRotation(GetUp(), up);
//
//localRotation = angle1 * localRotation;
//localRotation = Quaternion::Inverse(angle2) * localRotation;
//localRotation = worldRotation * localRotation;
//localRotation = originLocalRotation * localRotation;
//SetLocalRotation(localRotation);

///////////////////////////* Transform *///////////////////////////
void Transform::SetWorldTransform(const Matrix& transform)
{
	::memcpy(&mWorldTransform, &transform, sizeof(Matrix));
	::memcpy(&mLocalTransform, &transform, sizeof(Matrix));
	::memcpy(&mPrevTransform, &transform, sizeof(Matrix));

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

void Transform::ComputeWorldTransform(const Matrix* parentTransform)
{
	if (!parentTransform && mParent) {
		mWorldTransform = mLocalTransform * mParent->GetWorldTransform();
	}
	else {
		mWorldTransform = (parentTransform) ? (mLocalTransform * *parentTransform) : mLocalTransform;

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

	dxgi->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(mObjCBIndices[cnt]));
}

void Transform::NormalizeAxis()
{
	mLook = Vector3::Normalized(mLook);
	mRight = Vector3::Normalized(mUp.Cross(mLook));
	mUp = Vector3::Normalized(mLook.Cross(mRight));

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
	dxgi->SetGraphicsRoot32BitConstants(RootParam::Collider, XMMatrixTranspose(matrix), 0);
}

void Transform::UpdateShaderVars(const XMMATRIX& matrix)
{
	Matrix m;
	XMStoreFloat4x4(&m, matrix);
	UpdateShaderVars(m);
}