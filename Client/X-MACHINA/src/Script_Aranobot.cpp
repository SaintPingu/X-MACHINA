#include "stdafx.h"
#include "Script_Aranobot.h"

#include "InputMgr.h"

#include "Script_GroundObject.h"
#include "Script_EnemyManager.h"

#include "X-Engine.h"
#include "Timer.h"
#include "Object.h"
#include "AnimatorController.h"



void Script_Aranobot::Awake()
{
	base::Awake();

	mEnemyMgr->mRotationSpeed = 270.f;
	mEnemyMgr->mMoveSpeed = 2.8f;
	mEnemyMgr->mDetectionRange = 15.f;
}

void Script_Aranobot::Update()
{
	base::Update();

	if (KEY_PRESSED(VK_UP)) {
		mEnemyMgr->mController->SetValue("Jump", true);
	}
	else {
		mEnemyMgr->mController->SetValue("Jump", false);
	}

	if (KEY_PRESSED(VK_DOWN)) {
		mEnemyMgr->mController->SetValue("Landing", true);
	}
	else {
		mEnemyMgr->mController->SetValue("Landing", false);
	}

	if (KEY_PRESSED(VK_LEFT)) {
		mEnemyMgr->mController->SetValue("Dodge", true);
	}
	else {
		mEnemyMgr->mController->SetValue("Dodge", false);
	}
}