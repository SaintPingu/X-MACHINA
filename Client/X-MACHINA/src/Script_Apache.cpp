#include "pch.h"
#include "Script_Apache.h"

#include "Object.h"
#include "Timer.h"


void Script_Apache::Start()
{
	const auto& gameObject = mObject->GetObj<GameObject>();
	mMainRotor = gameObject->FindFrame("rotor");
	mTailRotor = gameObject->FindFrame("tail_rotor");
}

void Script_Apache::Animate()
{
	if (mMainRotor) {
		const float rotorSpeed = 360.f * 7.0f * DeltaTime();
		mMainRotor->Rotate(Vector3::Up(), rotorSpeed);
	}
	if (mTailRotor) {
		const float rotorSpeed = 360.f * 8.0f * DeltaTime();
		mTailRotor->Rotate(Vector3::Up(), rotorSpeed);
	}
}
