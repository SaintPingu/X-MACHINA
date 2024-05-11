#include "stdafx.h"
#include "Script_Ursacetus.h"

#include "InputMgr.h"

#include "Script_GroundObject.h"
#include "Script_EnemyManager.h"

#include "X-Engine.h"
#include "Timer.h"
#include "Object.h"

#include "AnimatorMotion.h"
#include "AnimatorController.h"



void Script_Ursacetus::Awake()
{
	base::Awake();

	mEnemyMgr->mRotationSpeed = 150.f;
	mEnemyMgr->mMoveSpeed = 3.5f;
	mEnemyMgr->mDetectionRange = 10.f;
	mEnemyMgr->mAttackName = "2HandsSmashAttack";
}

void Script_Ursacetus::Update()
{
	base::Update();
}

void Script_Ursacetus::Attack()
{
	
}