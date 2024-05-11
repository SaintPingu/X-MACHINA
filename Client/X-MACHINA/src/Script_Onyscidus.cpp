#include "stdafx.h"
#include "Script_Onyscidus.h"

#include "Script_EnemyManager.h"

void Script_Onyscidus::Awake()
{
	base::Awake();

	SetMaxHP(200);

	mEnemyMgr->mRotationSpeed = 150.f;
	mEnemyMgr->mMoveSpeed = 3.5f;
	mEnemyMgr->mDetectionRange = 10.f;
	mEnemyMgr->mAttackRange = 4.f;
	mEnemyMgr->mGetHitName = "BlockIdle";
	mEnemyMgr->mAttackName = "2HitComboClawsAttack";
}

void Script_Onyscidus::Update()
{
	base::Update();
}