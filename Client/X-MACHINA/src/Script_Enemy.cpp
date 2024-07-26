#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_EnemyManager.h"
#include "Script_GroundObject.h"
#include "Script_PheroObject.h"
#include "Script_DefaultEnemyBT.h"

#include "AnimatorMotion.h"
#include "AnimatorController.h"

#include "Timer.h"
#include "Object.h"

#include "XLManager.h"


void Script_Enemy::Awake()
{
	base::Awake();

	mEnemyMgr = mObject->AddComponent<Script_EnemyManager>();
	mObject->AddComponent<Script_GroundObject>();
	mObject->AddComponent<Script_PheroObject>();
	//mObject->AddComponent<Script_DefaultEnemyBT>();

	SetEnemyStat(mObject->GetName());
	SetMaxHP(mEnemyMgr->mStat.MaxHp);

	if (mEnemyMgr->mStat.Attack1AnimName != "None") {
		mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddEndCallback(std::bind(&Script_Enemy::AttackEndCallback, this));
	}
	if (mEnemyMgr->mStat.Attack2AnimName != "None") {
		mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack2AnimName)->AddEndCallback(std::bind(&Script_Enemy::AttackEndCallback, this));
	}
	if (mEnemyMgr->mStat.Attack3AnimName != "None") {
		mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddEndCallback(std::bind(&Script_Enemy::AttackEndCallback, this));
	}
}

void Script_Enemy::Update()
{
	base::Update();

	mObject->mObjectCB.HitRimFactor = max(mObject->mObjectCB.HitRimFactor - DeltaTime(), 0.f);
}

void Script_Enemy::Attack()
{
	mEnemyMgr->RemoveAllAnimation();
	mEnemyMgr->mController->SetValue("Attack", true);
}

void Script_Enemy::Death()
{
}

bool Script_Enemy::Hit(float damage, Object* instigator)
{
	bool res = base::Hit(damage, instigator);

	mObject->mObjectCB.HitRimFactor = 0.7f;
	
	if (nullptr != instigator) {
		mEnemyMgr->mTarget = instigator;
	}

	return res;
}

void Script_Enemy::SetEnemyStat(const std::string& modelName)
{
	XLManager::I->Set(modelName, mEnemyMgr->mStat);
}

void Script_Enemy::AttackCallback()
{
	if (!mEnemyMgr->mTarget) {
		return;
	}

	if (Vec3::Distance(mEnemyMgr->mTarget->GetPosition(), mObject->GetPosition()) <= mEnemyMgr->mStat.AttackRange) {
		auto liveObject = mEnemyMgr->mTarget->GetComponent<Script_LiveObject>();
		if (liveObject) {
			liveObject->Hit(mEnemyMgr->mStat.AttackRate, mObject);
		}
	}
}

void Script_Enemy::AttackEndCallback()
{
	mEnemyMgr->mController->SetValue("Attack", false);
	mEnemyMgr->mState = EnemyState::Idle;
}
