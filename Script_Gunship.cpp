#include "stdafx.h"
#include "Script_Gunship.h"

#include "Object.h"
#include "Timer.h"



void Script_Gunship::Start()
{
	mGameObject = mObject->GetObj<GameObject>();
	mMainRotor = mGameObject->FindFrame("Rotor");
	mTailRotor = mGameObject->FindFrame("Back_Rotor");
}

void Script_Gunship::Update()
{
	if (mMainRotor) {
		const float rotorSpeed = 360.f * 7.0f * DeltaTime();
		mMainRotor->Rotate(Vector3::Up(), rotorSpeed);
	}
	if (mTailRotor) {
		const float rotorSpeed = 360.f * 7.0f * DeltaTime();
		mTailRotor->Rotate(Vector3::Right(), rotorSpeed);
	}
}