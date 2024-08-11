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
#include "ClientNetwork/Contents/ClientNetworkManager.h"
#include "SoundMgr.h"

#include "XLManager.h"


void Script_Enemy::Awake()
{
	base::Awake();

	mEnemyMgr = mObject->AddComponent<Script_EnemyManager>();
	mEnemyMgr->mEnemy = this;
	mObject->AddComponent<Script_GroundObject>();
#ifndef SERVER_COMMUNICATION
	mObject->AddComponent<Script_PheroObject>();
	mObject->AddComponent<Script_DefaultEnemyBT>();
#endif

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
	if (mEnemyMgr->mStat.DeathAnimName != "None") {
		mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.DeathAnimName)->AddEndCallback(std::bind(&Script_Enemy::DeathEndCallback, this));
	}
}

void Script_Enemy::Update()
{
	base::Update();

	mObject->mObjectCB.HitRimFactor = std::max(mObject->mObjectCB.HitRimFactor - DeltaTime(), 0.f);

	// TODO : 임의로 발표를 위해 여기에 둠 추후에 변경해야 한다.
	if (IsDead()) {
		mEnemyMgr->mState = EnemyState::Death;

		mAccTime += DeltaTime();

		mEnemyMgr->RemoveAllAnimation();
		mEnemyMgr->mController->SetValue("Death", true);

		//ExecuteCallback();

		if (mAccTime >= mRemoveTime) {
			mObject->mObjectCB.HitRimFactor = 0.7f;
			mObject->Destroy();
		}
	}
}

void Script_Enemy::StartAttack()
{
	if (mCurrAttackCnt == AttackType::None) {
		mCurrAttackCnt = AttackType::BasicAttack;
	}

	mEnemyMgr->RemoveAllAnimation();
	mEnemyMgr->mState = EnemyState::Attack;
	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt);
}

void Script_Enemy::Attack()
{
}

void Script_Enemy::Dead()
{
	base::Dead();

	if(mDeathSound != "") {
		SoundMgr::I->Play("Enemy", mDeathSound);
	}
}

void Script_Enemy::Detect()
{
	if (mDetectSound != "") {
		SoundMgr::I->Play("Enemy", mDetectSound);
	}
}



bool Script_Enemy::Hit(float damage, Object* instigator)
{
	if (IsDead()) {
		return false;
	}

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

void Script_Enemy::SetCurrAttackCnt(int attackCnt)
{
	mCurrAttackCnt = attackCnt;
	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt, true);
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

		if (mAttackSound != "") {
			SoundMgr::I->Play("Enemy", mAttackSound);
		}
	}
}

void Script_Enemy::AttackEndCallback()
{
}

void Script_Enemy::DeathEndCallback()
{
	mEnemyMgr->mController->GetCrntMotion()->SetSpeed(0.f);
}