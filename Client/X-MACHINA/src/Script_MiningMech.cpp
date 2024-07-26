#include "stdafx.h"
#include "Script_MiningMech.h"

#include "Script_EnemyManager.h"
#include "Script_MainCamera.h"

#include "Component/Collider.h"
#include "Component/Camera.h"

#include "Script_AbilityHolder.h"
#include "CircleIndicator.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"


void Script_MiningMech::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_MiningMech::DiggerAttackCallback, this), 15);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack2AnimName)->AddCallback(std::bind(&Script_MiningMech::DrillAttackCallback, this), 15);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddCallback(std::bind(&Script_MiningMech::SmashAttackCallback, this), 20);
}

void Script_MiningMech::Attack()
{
	mEnemyMgr->RemoveAllAnimation();
	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt);
}

void Script_MiningMech::DiggerAttackCallback()
{
}

void Script_MiningMech::DrillAttackCallback()
{
}

void Script_MiningMech::SmashAttackCallback()
{
}

void Script_MiningMech::AttackEndCallback()
{
	++mCurrAttackCnt;
	mCurrAttackCnt %= AttackTypeCount;
	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt, true);
	mEnemyMgr->mState = EnemyState::Idle;
}