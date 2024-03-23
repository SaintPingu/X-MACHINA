#include "pch.h"
#include "Component/Script_MainCamera.h"

#include "Object.h"
#include "Timer.h"
#include "Component/Camera.h"
#include "Scene.h"

#include "X-Engine.h"


void Script_MainCamera::SetCameraOffset(const Vec3& offset)
{
	mOffset = offset;
	mainCamera->SetOffset(mOffset);
	LookPlayer();
	mainCamera->UpdateViewMtx();
}


void Script_MainCamera::Start()
{
	mPlayer = engine->GetPlayer();
	mObject->SetPosition(Vector3::Add(mPlayer->GetPosition(), Vector3::One()));
	Init();
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

void Script_MainCamera::Init()
{
	constexpr float maxPlaneDistance = 1000.f;
	mTimeLag = 0.1f;
	SetCameraOffset(Vec3(0.f, 5.f, -10.f));	// back
	mainCamera->SetProjMtx(0.01f, maxPlaneDistance, 80.f);
}


void Script_MainCamera::LookPlayer()
{
	if (mPlayer) {
		mainCamera->LookAt(mPlayer->GetPosition(), mPlayer->GetUp());
		mainCameraObject->Rotate(-10, 0, 0);
	}
}