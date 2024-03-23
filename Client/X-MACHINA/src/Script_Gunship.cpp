#include "pch.h"
#include "Script_Gunship.h"

#include "Object.h"
#include "Timer.h"


void Script_Gunship::Start()
{
	const auto& gameObject = mObject->GetObj<GameObject>();
	mMainRotor = gameObject->FindFrame("Rotor");
	mTailRotor = gameObject->FindFrame("Back_Rotor");
}

void Script_Gunship::Animate()
{
	mObject->Rotate(0, 90 * DeltaTime(), 0);
	if (mMainRotor) {
		const float rotorSpeed = 360.f * 7.0f * DeltaTime();
		mMainRotor->Rotate(Vector3::Up(), rotorSpeed);
	}
	if (mTailRotor) {
		const float rotorSpeed = 360.f * 7.0f * DeltaTime();
		mTailRotor->Rotate(Vector3::Right(), rotorSpeed);
	}
}