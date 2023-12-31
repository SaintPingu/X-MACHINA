#include "stdafx.h"
#include "Script_MainCamera.h"

#include "Object.h"
#include "Timer.h"
#include "Camera.h"
#include "Scene.h"


void Script_MainCamera::SetCameraOffset(const Vec3& offset)
{
	mOffset = offset;
	mainCamera->SetOffset(mOffset);
	LookPlayer();
	mainCamera->UpdateViewMtx();
}


void Script_MainCamera::Start()
{
	mPlayer = scene->GetPlayer();
	ChangeCameraMode(CameraMode::Third);
}

void Script_MainCamera::Update()
{
	Vec4x4 rotationMtx = Matrix4x4::Identity();
	const Vec3 right   = mPlayer->GetRight();
	const Vec3 up      = mPlayer->GetUp();
	const Vec3 look    = mPlayer->GetLook();

	rotationMtx._11 = right.x; rotationMtx._21 = up.x; rotationMtx._31 = look.x;
	rotationMtx._12 = right.y; rotationMtx._22 = up.y; rotationMtx._32 = look.y;
	rotationMtx._13 = right.z; rotationMtx._23 = up.z; rotationMtx._33 = look.z;

	const Vec3 offset   = Vector3::TransformCoord(mOffset, rotationMtx);
	const Vec3 position = Vector3::Add(mPlayer->GetPosition(), offset);
	Vec3 dir            = Vector3::Subtract(position, mObject->GetPosition());

	const float timeScale = (mTimeLag) ? DeltaTime() * (1.f / mTimeLag) : 1.f;
	const float length    = Vector3::Length(dir);
	dir                   = Vector3::Normalize(dir);

	float distance = length * timeScale;


	if (distance > length) {
		distance = length;
	}

	if (length < 0.01f) {
		distance = length;
	}

	if (distance > 0) {
		mainCameraObject->Translate(dir, distance);
		LookPlayer();
		mainCamera->UpdateViewMtx();
	}

}

void Script_MainCamera::UpdateHeight()
{
	constexpr float minHeight = 2.0f;

	if (mObject->GetPosition().y <= minHeight) {
		mObject->SetPositionY(minHeight);
		LookPlayer();
	}

	Vec3 camPos        = mObject->GetPosition();
	const float height = scene->GetTerrainHeight(camPos.x, camPos.z) + minHeight;

	if (camPos.y <= height) {
		camPos.y = height;
		mObject->SetPosition(camPos);
	}
}

void Script_MainCamera::RotateOffset(const Vec3& axis, float angle)
{
	const Vec3 cameraOffset    = mainCamera->GetOffset();
	const Matrix mtxRotation   = XMMatrixRotationAxis(_VECTOR(axis), XMConvertToRadians(angle));
	const Vec3 newCameraOffset = Vector3::TransformCoord(cameraOffset, mtxRotation);

	mainCamera->SetOffset(newCameraOffset);
}

void Script_MainCamera::ChangeCameraMode(CameraMode mode)
{
	constexpr float maxPlaneDistance = 1000.f;

	if (mCameraMode == mode) {
		return;
	}

	switch (mode) {
	case CameraMode::Third:
		mTimeLag = 0.1f;
		SetCameraOffset(Vec3(0.f, 15.f, -30.f));
		mainCamera->SetProjMtx(1.01f, maxPlaneDistance, gkAspectRatio, 80.f);
		mainCamera->SetViewport(0, 0, gkFrameBufferWidth, gkFrameBufferHeight, 0.f, 1.f);
		mainCamera->SetScissorRect(0, 0, gkFrameBufferWidth, gkFrameBufferHeight);
		break;
	default:
		assert(0);
		break;
	}

	mCameraMode = mode;
}


void Script_MainCamera::LookPlayer()
{
	if (mPlayer) {
		mainCamera->LookAt(mPlayer->GetPosition(), mPlayer->GetUp());
	}
}