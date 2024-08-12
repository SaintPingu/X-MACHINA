#include "stdafx.h"
#include "Script_Deus_Phase_1.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

void Script_Deus_Phase_1::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Deus_Phase_1::MeleeAttackCallback, this), 16);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack2AnimName)->AddStartCallback(std::bind(&Script_Deus_Phase_1::RangeAttackCallback, this));
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddStartCallback(std::bind(&Script_Deus_Phase_1::ExplodeAttackCallback, this));

}

void Script_Deus_Phase_1::Start()
{
	base::Start();

	mCurrAttackCnt = static_cast<int>(AttackType::BasicAttack);
}

void Script_Deus_Phase_1::MeleeAttackCallback()
{
}

void Script_Deus_Phase_1::RangeAttackCallback()
{
}

void Script_Deus_Phase_1::ExplodeAttackCallback()
{
}

void Script_Deus_Phase_1::AttackEndCallback()
{
	if (mEnemyMgr->mState != EnemyState::Attack) {
		mEnemyMgr->mController->SetValue("Attack", ScriptDeusPhase1AttackType::None);
		return;
	}

	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt);

	if (mCurrAttackCnt >= ScriptDeusPhase1AttackType::BasicAttack) {
		mEnemyMgr->mController->SetValue("Attack", ScriptDeusPhase1AttackType::None);
		mEnemyMgr->mState = EnemyState::Idle;
	}
}
