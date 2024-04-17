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
	mEnemyMgr->mDetectionRange = 10.f;
}

void Script_Aranobot::Update()
{
	base::Update();

	//if (KEY_PRESSED(VK_UP)) {
	//	mEnemyMgr->mController->SetValue("Walk", true);
	//}
	//else {
	//	mEnemyMgr->mController->SetValue("Walk", false);
	//}

	//if (KEY_PRESSED(VK_DOWN)) {
	//	mEnemyMgr->mController->SetValue("Attack", true);
	//}
	//else {
	//	mEnemyMgr->mController->SetValue("Attack", false);
	//}

	//if (KEY_PRESSED(VK_LEFT)) {
	//	mEnemyMgr->mController->SetValue("Hit", true);
	//}
	//else {
	//	mEnemyMgr->mController->SetValue("Hit", false);
	//}
}