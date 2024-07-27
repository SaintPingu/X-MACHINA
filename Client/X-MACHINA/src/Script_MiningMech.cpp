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

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddStartCallback(std::bind(&Script_MiningMech::SmashAttackStartCallback, this));
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddCallback(std::bind(&Script_MiningMech::SmashAttackCallback, this), 20);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddCallback(std::bind(&Script_MiningMech::SmashAttackEndCallback, this), 49);
}

void Script_MiningMech::Start()
{
	base::Start();

	mRectangleIndicator = mObject->AddComponent<Script_StateAbilityHolder>();
	mRectangleIndicator.lock()->SetAbility(0, std::make_shared<AttackIndicator>(1.8f, "RectangleIndicator"));
}

void Script_MiningMech::LateUpdate()
{
	base::LateUpdate();

	mEnemyMgr->mController->SetValue("IsAttack", false);
}

void Script_MiningMech::Attack()
{
	mEnemyMgr->RemoveAllAnimation();
	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt);
	mEnemyMgr->mController->SetValue("IsAttack", true);
}

void Script_MiningMech::DiggerAttackCallback()
{
}

void Script_MiningMech::DrillAttackCallback()
{
}

void Script_MiningMech::SmashAttackStartCallback()
{
	if (mEnemyMgr->mController->GetParamValue<bool>("IsAttack")) {
		mRectangleIndicator.lock()->SetActive(true);
	}
}

void Script_MiningMech::SmashAttackCallback()
{
}

void Script_MiningMech::SmashAttackEndCallback()
{
}

void Script_MiningMech::AttackEndCallback()
{
	if (mCurrAttackCnt == static_cast<int>(AttackType::SmashAttack)) {
		mRectangleIndicator.lock()->SetActive(false);
	}
	++mCurrAttackCnt;
	mCurrAttackCnt %= AttackTypeCount;

	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt, true);
	mEnemyMgr->mController->SetValue("IsAttack", false);
	mEnemyMgr->mState = EnemyState::Idle;
}