#include "stdafx.h"
#include "Script_AdvancedCombatDroid_5.h"

#include "Script_EnemyManager.h"

void Script_AdvancedCombatDroid_5::Awake()
{
	base::Awake();

	SetMaxHP(200);

	mEnemyMgr->mRotationSpeed = 150.f;
	mEnemyMgr->mMoveSpeed = 2.5f;
	mEnemyMgr->mDetectionRange = 15.f;
	mEnemyMgr->mAttackRange = 4.f;
	mEnemyMgr->mGetHitName = "IdleCombat";
	mEnemyMgr->mAttackName = "2HitComboUnarmed";
	mEnemyMgr->mDeathName = "DeathFrontCombat";
}