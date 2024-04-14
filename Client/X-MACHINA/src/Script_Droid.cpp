#include "stdafx.h"
#include "Script_Droid.h"

#include "Script_GroundObject.h"

#include "X-Engine.h"
#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"


void Script_Droid::Awake()
{
	base::Awake();

	Transform* gunPos = mObject->FindFrame("WeaponAction");
	mGun = scene->Instantiate("SM_SciFiAssaultPistol");
	gunPos->SetChild(mGun);
}

void Script_Droid::Update()
{
	return;
	const float kMinDistance = 0.1f;

	float distance = (mObject->GetPosition() - mPlayer->GetPosition()).Length();
	if (distance < mDetectionRange && distance > kMinDistance) {
		mTarget = mPlayer;
	}
	else if (mTarget) {
		mTarget = nullptr;
		mController->SetValue("Walk", false);
	}

	if (mTarget) {
		mObject->RotateTargetAxisY(mTarget->GetPosition(), mRotationSpeed * DeltaTime());
		mObject->Translate(mObject->GetLook(), mMoveSpeed * DeltaTime());
		if (mController) {
			mController->SetValue("Walk", true);
		}
	}
}

void Script_Droid::OnDestroy()
{
	mGun->OnDestroy();
}