#include "Component/Transform.h"
#include "EnginePch.h"

#include "Component/Transform.h"

#include "Timer.h"
#include "DXGIMgr.h"
#include "FrameResource.h"
#include "Texture.h"

#pragma region Getter
Quat Transform::GetLocalRotation() const
{
	return Quat::CreateFromRotationMatrix(mLocalTransform);
}

Quat Transform::GetRotation() const
{
	return Quat::CreateFromRotationMatrix(mWorldTransform);
}

Vec3 Transform::GetDirection(Dir dir, float distance) const
{
	if (dir == Dir::None) {
		return Vector3::Zero;
	}

	Vec3 result{};

	result += static_cast<bool>(dir & Dir::Front) *  mLook;
	result += static_cast<bool>(dir & Dir::Back)  * -mLook;
	result += static_cast<bool>(dir & Dir::Right) *  mRight;
	result += static_cast<bool>(dir & Dir::Left)  * -mRight;
	result += static_cast<bool>(dir & Dir::Up)    *  mUp;
	result += static_cast<bool>(dir & Dir::Down)  * -mUp;

	return Vector3::Normalized(result);
}

Vec3 Transform::GetWorldDirection(Dir dir)
{
	if (dir == Dir::None) {
		return Vector3::Zero;
	}

	Vec3 result{};
	result += static_cast<bool>(dir & Dir::Front) * Vector3::Forward;
	result += static_cast<bool>(dir & Dir::Back)  * Vector3::Backward;
	result += static_cast<bool>(dir & Dir::Right) * Vector3::Right;
	result += static_cast<bool>(dir & Dir::Left)  * Vector3::Left;
	result += static_cast<bool>(dir & Dir::Up)    * Vector3::Up;
	result += static_cast<bool>(dir & Dir::Down)  * Vector3::Down;

	return Vector3::Normalized(result);
}

float Transform::GetYAngle() const
{
	return Vector3::SignedAngle(Vector3::Forward, GetLook().xz(), Vector3::Up);
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

void Transform::SetPosition(const Vec2& pos)
{
	SetPosition(pos.x, pos.y, 0.f);
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

void Transform::SetChild(rsptr<Transform> child, bool isKeepLocalTransform)
{
	if (child) {
		child->mParent = this;
	}
	else {
		mChild = nullptr;
		return;
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
				child->mSibling = nullptr;
			}
			mChild->mSibling = child;
		}
	}
	else {
		mChild = child;
	}

	if (!isKeepLocalTransform) {
		child->SetLocalTransform(child->GetWorldTransform() * Matrix4x4::Inverse(mWorldTransform));
		child->ComputeWorldTransform();
	}
}

sptr<Transform> Transform::DetachParent(bool isKeepLocalTransform)
{
	if (!mParent) {
		return nullptr;
	}
	const Matrix& worldTransform = GetWorldTransform();
	if (mChild) {
		mChild->mParent = mParent;
	}

	sptr<Transform> crnt = mParent->mChild;
	if (!crnt) {
		return nullptr;
	}
	Transform* prev      = crnt.get();

	if (crnt.get() == this) {		// parent's child is this
		mParent->SetChild(nullptr);
	}
	else {
		while (crnt.get() != this) {
			prev = crnt.get();
			crnt = crnt->mSibling;
			if (!crnt) {
				return nullptr;
			}
		}

		// crnt == this
		if (prev != crnt.get()) {
			prev->mSibling = crnt->mSibling;
		}
	}

	mParent = nullptr;
	if (!isKeepLocalTransform) {
		SetLocalTransform(worldTransform);
	}
	ComputeWorldTransform();
	return crnt;
}

void Transform::SetLocalTransform(const Matrix& transform, bool isComputeWorldTransform)
{
	mLocalTransform = transform;
	mPrevTransform = transform;
	UpdateAxis(isComputeWorldTransform);
}
void Transform::SetTexture(rsptr<Texture> texture)
{
	if (texture) {
		mMatIndex = texture->GetSrvIdx();
	}
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

void Transform::RotateToDir(const Vec3& dir)
{
	Vec3 from = GetLook().xz();
	Vec3 to = dir.xz();

	float angle = Vector3::SignedAngle(from, to, Vector3::Up);

	RotateGlobal(Vector3::Up, angle);
}

void Transform::RotateGlobal(const Vec3& axis, float angle)
{
	RotateGlobal(XMQuaternionRotationAxis(_VECTOR(axis), XMConvertToRadians(angle)));
}

void Transform::RotateGlobal(const Vec3& eulerAngles)
{
	RotateGlobal(_VECTOR4(eulerAngles.ToQuaternion()));
}

void Transform::RotateGlobal(const XMVECTOR& quatAngles)
{
	//// <Algorithm>
	//// 1. 기존 회전값과 월드 회전값을 미리 저장한다.
	//// 2. 월드 회전값의 역으로 회전하여 좌표축을 정상(왼손 좌표계)으로 되돌린다.
	//// 3. 해당 좌표축은 월드 공간이다. 원하는 축으로 회전한다.
	//// 4. 1에서 저장한 월드 회전값과 기존 회전값을 적용한다.
	
	// 1 //
	XMVECTOR originLocalRotation = _VECTOR4(GetLocalRotation());
	SetLocalRotation(Quat::Identity);
	XMVECTOR worldRotation = _VECTOR4(GetRotation());

	// 2 //
	XMVECTOR localRotation = XMQuaternionInverse(worldRotation);
	localRotation = XMQuaternionMultiply(quatAngles, localRotation);

	// 3 //
	localRotation = XMQuaternionMultiply(worldRotation, localRotation);

	// 4 //
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
	const float angle = Vector3::SignedAngle(GetLook().xz(), (target - GetPosition()).xz(), Vector3::Up);

	constexpr float minAngle = 2.f;
	if (fabs(angle) < minAngle) {
		return false;
	}

	constexpr float maxAngle = 90.f; // 90도 이상일 때 최대 속도
	const float t = std::clamp(fabs(angle) / maxAngle, 0.f, 1.f);
	rotationSpeed *= t;

	const float rotationValue = Math::Sign(angle) * rotationSpeed;
	float rotationAngle       = rotationValue * DeltaTime();

	if (fabs(rotationAngle) > fabs(angle)) {
		rotationAngle = angle;
	}

	Rotate(Vector3::Up, rotationAngle);
	return true;
}

void Transform::SetLocalRotation(const Vec3& eulerAngles)
{
	SetLocalRotation(eulerAngles.ToQuaternion());
}

void Transform::SetLocalRotation(const Vec4& quaternion)
{
	Matrix4x4::SetRotation(mLocalTransform, quaternion);

	UpdateAxis();
}

void Transform::ResetRotation(float yAngle)
{
	SetLocalRotation(Quat::Identity);
	Rotate(0, yAngle, 0);
}

void Transform::LookTo(const Vec3& lookTo, const Vec3& up)
{
	SetAxis(Matrix4x4::LookToLH(GetPosition(), lookTo, up, false));
}

void Transform::LookAt(const Vec3& lookAt, const Vec3& up)
{
	SetAxis(Matrix4x4::LookAtLH(mPosition, lookAt, up, false));
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

std::vector<Transform*> Transform::GetAllTransforms()
{
	std::vector<Transform*> result;
	GetAllTransforms(result, this);
	return result;
}

void Transform::GetAllTransforms(std::vector<Transform*>& out, Transform* current)
{
	out.push_back(current);

	if (current->mSibling) {
		GetAllTransforms(out, current->mSibling.get());
	}
	if (current->mChild) {
		GetAllTransforms(out, current->mChild.get());
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
		FRAME_RESOURCE_MGR->ReturnIndex(index, BufferType::Object);
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

void Transform::UpdateShaderVars(ObjectConstants& objectCB, const int cnt) const
{
	if (mObjCBIndices.size() <= cnt) {
		return;
	}

	// 실제 사용 횟수를 저장한다.
	if (mObjCBCount <= cnt) {
		mObjCBCount = cnt + 1;
	}

	if (mMatIndex >= 0) {
		objectCB.AltDiffuseTextureIndex = mMatIndex;
	}

	FRAME_RESOURCE_MGR->CopyData(mObjCBIndices[cnt], objectCB);

	DXGIMgr::I->SetGraphicsRootConstantBufferView(RootParam::Object, FRAME_RESOURCE_MGR->GetObjCBGpuAddr(mObjCBIndices[cnt]));
}

void Transform::UpdateShaderVars(const int cnt, const int matIndex) const
{
	// 실제 사용 횟수를 저장한다.
	if (mObjCBCount <= cnt) {
		mObjCBCount = cnt + 1;
	}

	ObjectConstants objectConstants;
	objectConstants = mObjectCB;
	objectConstants.MtxWorld = XMMatrixTranspose(_MATRIX(GetWorldTransform()));
	objectConstants.MatIndex = matIndex;

	FRAME_RESOURCE_MGR->CopyData(mObjCBIndices[cnt], objectConstants);

	DXGIMgr::I->SetGraphicsRootConstantBufferView(RootParam::Object, FRAME_RESOURCE_MGR->GetObjCBGpuAddr(mObjCBIndices[cnt]));
}

void Transform::NormalizeAxis()
{
	mLook = Vector3::Normalized(mLook);
	mRight = Vector3::Normalized(mUp.Cross(mLook));
	mUp = Vector3::Normalized(mLook.Cross(mRight));

	UpdateLocalTransform(false);
}

void Transform::DoAllTransforms(const std::function<void(Transform*)>& processFunc)
{
	processFunc(this);

	if (mChild) {
		mChild->DoAllTransforms(processFunc);
	}
	if (mSibling) {
		mSibling->DoAllTransforms(processFunc);
	}
}

void Transform::DoAllChilds(const std::function<void(Transform*)>& processFunc)
{
	processFunc(this);

	if (mChild) {
		mChild->DoAllTransforms(processFunc);
	}
}

Transform* Transform::GetRoot()
{
	if (!mParent) {
		return this;
	}

	return mParent->GetRoot();
}

void Transform::UpdateColliderShaderVars(const Matrix& matrix, const Vec4& color)
{
	Matrix m;
	XMStoreFloat4x4(&m, matrix);
	DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::Collider, XMMatrixTranspose(matrix), 0);
	DXGIMgr::I->SetGraphicsRoot32BitConstants(RootParam::Collider, color, 16);
}