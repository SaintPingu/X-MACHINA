#include "stdafx.h"
#include "Script_Droid.h"

#include "Script_GroundObject.h"
#include "Script_EnemyManager.h"

#include "X-Engine.h"
#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"


void Script_Droid::Awake()
{
	base::Awake();

	mEnemyMgr->mRotationSpeed = 270.f;
	mEnemyMgr->mMoveSpeed = 1.8f;
	mEnemyMgr->mDetectionRange = 10.f;

	Transform* gunPos = mObject->FindFrame("WeaponAction");
	mGun = scene->Instantiate("SM_SciFiAssaultPistol");
	gunPos->SetChild(mGun);
}

void Script_Droid::Update()
{
	base::Update();
}

void Script_Droid::OnDestroy()
{
	mGun->OnDestroy();
}