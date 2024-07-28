#include "stdafx.h"
#include "Script_Anglerox.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"


void Script_Anglerox::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Anglerox::AttackCallback, this), 10);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Anglerox::AttackCallback, this), 32);

	SetDetectSound("Anglerox Detect");
	SetAttackSound("Anglerox Attack");
}
