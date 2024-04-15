#include "stdafx.h"
#include "Script_MainCamera.h"

#include "Object.h"
#include "Timer.h"
#include "Component/Camera.h"
#include "Scene.h"

#include "X-Engine.h"


void Script_MainCamera::SetCameraOffset(const Vec3& offset)
{
	mMainOffset = offset;
	mainCamera->SetOffset(mMainOffset);
}


void Script_MainCamera::Start()
{
	mPlayer = engine->GetPlayer();
	Init();
}

void Script_MainCamera::Update()
{
	mObject->SetPosition(mPlayer->GetPosition() + mMainOffset + mExtraOffset);
}

void Script_MainCamera::Init()
{
	constexpr float maxPlaneDistance = 100.f;
	SetCameraOffset(Vec3(0.f, 12.f, -7.f));
	mObject->SetPosition(mPlayer->GetPosition() + mMainOffset);
	LookPlayer();

	mainCamera->SetProjMtx(0.01f, maxPlaneDistance, 60.f);
}


void Script_MainCamera::LookPlayer()
{
	if (mPlayer) {
		mainCamera->LookAt(mPlayer->GetPosition(), mPlayer->GetUp());
	}
}