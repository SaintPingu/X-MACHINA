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
	mObject->SetPosition(Vector3::Add(mPlayer->GetPosition(), Vector3::One()));
	ChangeCameraMode(CameraMode::Third);
}

void Script_MainCamera::Update()
{
	Vec4x4 rotationMtx = Matrix4x4::Identity();
	const Vec3 right   = mPlayer->GetRight();
	const Vec3 up      = mPlayer->GetUp();
	const Vec3 look    = mPlayer->GetLook();

	std::memcpy(&rotationMtx._11, &right, sizeof(Vec3));
	std::memcpy(&rotationMtx._21, &up,	  sizeof(Vec3));
	std::memcpy(&rotationMtx._31, &look,  sizeof(Vec3));

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
		mObject->Translate(dir, distance);
		UpdateHeight();
		LookPlayer();
		mainCamera->UpdateViewMtx();
	}

}

void Script_MainCamera::UpdateHeight()
{
	return;
	constexpr float kMinHeight = 2.0f;

	if (mObject->GetPosition().y <= kMinHeight) {
		mObject->SetPositionY(kMinHeight);
		LookPlayer();
	}

	Vec3 camPos = mObject->GetPosition();
	const float height = scene->GetTerrainHeight(camPos.x, camPos.z) + kMinHeight;

	if (camPos.y <= height) {
		camPos.y = height;
		mObject->SetPosition(camPos);
	}
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
		SetCameraOffset(Vec3(0.f, 12.f, -1.f));
		//SetCameraOffset(Vec3(1.5f, 2.5f, 1.5f));
		//SetCameraOffset(Vec3(0.f, 10.f, -30.f));
		//SetCameraOffset(Vec3(0.f, 30.f, -50.f));
		mainCamera->SetProjMtx(1.01f, maxPlaneDistance, gkAspectRatio, 80.f);
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
		mainCameraObject->Rotate(-10, 0, 0);
	}
}