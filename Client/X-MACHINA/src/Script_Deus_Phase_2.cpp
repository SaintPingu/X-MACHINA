#include "stdafx.h"
#include "Script_Deus_Phase_2.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

#include "Object.h"
#include "BattleScene.h"

void Script_Deus_Phase_2::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Deus_Phase_2::MeleeAttackCallback, this), 16);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack2AnimName)->AddStartCallback(std::bind(&Script_Deus_Phase_2::RangeAttackCallback, this));
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddStartCallback(std::bind(&Script_Deus_Phase_2::ExplodeAttackCallback, this));
}

void Script_Deus_Phase_2::MeleeAttackCallback()
{
}

void Script_Deus_Phase_2::RangeAttackCallback()
{
}

void Script_Deus_Phase_2::ExplodeAttackCallback()
{
}
