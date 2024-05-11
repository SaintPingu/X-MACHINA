#include "stdafx.h"
#include "Script_Ursacetus.h"

#include "Script_EnemyManager.h"

void Script_Ursacetus::Awake()
{
	base::Awake();

	SetMaxHP(1000);

	mEnemyMgr->mRotationSpeed = 150.f;
	mEnemyMgr->mMoveSpeed = 3.5f;
	mEnemyMgr->mDetectionRange = 20.f;
	mEnemyMgr->mAttackRange = 7.5f;
	mEnemyMgr->mGetHitName = "GetHitFront";
	mEnemyMgr->mAttackName = "2HandsSmashAttack";
}

void Script_Ursacetus::Update()
{
	base::Update();
}

void Script_Ursacetus::Attack()
{
	
}