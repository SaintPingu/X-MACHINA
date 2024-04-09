#include "stdafx.h"
#include "Script_MainCamera.h"

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
}


void Script_MainCamera::Start()
{
	mPlayer = engine->GetPlayer();
	mObject->SetPosition(mPlayer->GetPosition() + Vector3::One());
	Init();
}

void Script_MainCamera::Update()
{
	const Vec3 position = mPlayer->GetPosition() + mOffset;
	Vec3 dir            = position - mObject->GetPosition();

	const float timeScale = (mTimeLag) ? DeltaTime() * (1.f / mTimeLag) : 1.f;
	const float length    = dir.Length();
	dir.Normalize();

	float distance = length * timeScale;
	if (distance > 0) {
		mObject->Translate(dir, distance);
		LookPlayer();
	}

}

void Script_MainCamera::Init()
{
	constexpr float maxPlaneDistance = 1000.f;
	mTimeLag = 0.1f;
	SetCameraOffset(Vec3(0.f, 12.0f, -4.f));
	mainCamera->SetProjMtx(0.01f, maxPlaneDistance, 80.f);
}


void Script_MainCamera::LookPlayer()
{
	if (mPlayer) {
		mainCamera->LookAt(mPlayer->GetPosition(), mPlayer->GetUp());
	}
}