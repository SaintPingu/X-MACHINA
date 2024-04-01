#include "stdafx.h"
#include "Script_Enemy.h"

#include "Timer.h"

#include "Script_GroundObject.h"

#include "X-Engine.h"
#include "Scene.h"
#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"



void Script_Enemy::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_GroundObject>();
	Transform* gunPos = mObject->FindFrame("WeaponAction");
	mGun = scene->Instantiate("SM_SciFiAssaultPistol");
	gunPos->SetChild(mGun);
	mObject->SetTag(ObjectTag::Enemy);

	mPlayer = engine->GetPlayer();
	mController = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
}

void Script_Enemy::Start()
{
	base::Start();
}

void Script_Enemy::Update()
{
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
		mObject->RotateTargetAxisY(mTarget->GetPosition(), mkRotationSpeed * DeltaTime());
		mObject->Translate(mObject->GetLook(), mMoveSpeed * DeltaTime());
		if (mController) {
			mController->SetValue("Walk", true);
		}
	}
}

void Script_Enemy::OnDestroy()
{
	mGun->OnDestroy();
}