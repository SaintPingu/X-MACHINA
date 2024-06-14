#include "stdafx.h"
#include "Script_AdvancedCombatDroid_5.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

void Script_AdvancedCombatDroid_5::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.AttackAnimName)->AddCallback(std::bind(&Script_AdvancedCombatDroid_5::AttackCallback, this), 3);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.AttackAnimName)->AddCallback(std::bind(&Script_AdvancedCombatDroid_5::AttackCallback, this), 20);
}
