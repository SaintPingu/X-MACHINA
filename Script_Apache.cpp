#include "stdafx.h"
#include "Script_Apache.h"
#include "Object.h"
#include "Timer.h"

void Script_Apache::Start()
{
	mGameObject = mObject->Object<CGameObject>();
	mGameObject->SetFlyable(true);
	mMainRotorFrame = mGameObject->FindFrame("rotor");
	mTailRotorFrame = mGameObject->FindFrame("tail_rotor");
}


void Script_Apache::Update()
{
	if (mMainRotorFrame) {
		const float rotorSpeed = 360.0f * 7.0f * DeltaTime();
		mMainRotorFrame->Rotate(Vector3::Up(), rotorSpeed);
	}
	if (mTailRotorFrame) {
		const float rotorSpeed = 360.0f * 8.0f * DeltaTime();
		mTailRotorFrame->Rotate(Vector3::Up(), rotorSpeed);
	}
}
