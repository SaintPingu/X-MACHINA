#include "stdafx.h"
#include "Script_Gunship.h"
#include "Object.h"
#include "Timer.h"

void Script_Gunship::Start()
{
	mGameObject = mObject->Object<CGameObject>();
	mGameObject->SetFlyable(true);
	mMainRotorFrame = mGameObject->FindFrame("Rotor");
	mTailRotorFrame = mGameObject->FindFrame("Back_Rotor");
}


void Script_Gunship::Update()
{
	if (mMainRotorFrame) {
		const float rotorSpeed = 360.0f * 7.0f * DeltaTime();
		mMainRotorFrame->Rotate(Vector3::Up(), rotorSpeed);
	}
	if (mTailRotorFrame) {
		const float rotorSpeed = 360.0f * 7.0f * DeltaTime();
		mTailRotorFrame->Rotate(Vector3::Right(), rotorSpeed);
	}
}
