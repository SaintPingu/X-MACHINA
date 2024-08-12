#include "stdafx.h"
#include "Script_MiningMech.h"

#include "Script_EnemyManager.h"
#include "Script_MainCamera.h"

#include "Component/Collider.h"
#include "Component/Camera.h"

#include "Script_Ability_AttackIndicator.h"

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
	//mCurrAttackCnt = static_cast<int>(AttackType::BasicAttack);

	mIndicator = mObject->AddComponent<Script_Ability_AttackIndicator>();
	mIndicator.lock()->Init(1.8f, "RectangleIndicator");
}

void Script_MiningMech::LateUpdate()
{
	base::LateUpdate();
}
//
//void Script_MiningMech::Attack()
//{
//	mEnemyMgr->RemoveAllAnimation();
//	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt);
//}

void Script_MiningMech::DiggerAttackCallback()
{
}

void Script_MiningMech::DrillAttackCallback()
{
}

void Script_MiningMech::SmashAttackStartCallback()
{
	mIndicator.lock()->SetActive(true);
}

void Script_MiningMech::SmashAttackCallback()
{
}

void Script_MiningMech::SmashAttackEndCallback()
{
}

void Script_MiningMech::AttackEndCallback()
{
	if (mEnemyMgr->mState != EnemyState::Attack) {
		mEnemyMgr->mController->SetValue("Attack", MiningMechAttackType::None);
		return;
	}

	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt);
	std::cout << mCurrAttackCnt << std::endl;

	if (mCurrAttackCnt >= MiningMechAttackType::BasicAttack) {
		mEnemyMgr->mController->SetValue("Attack", MiningMechAttackType::None);
		mEnemyMgr->mState = EnemyState::Idle;
	}
}