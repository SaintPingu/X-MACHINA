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
}

void Script_Aranobot::Attack()
{
}
