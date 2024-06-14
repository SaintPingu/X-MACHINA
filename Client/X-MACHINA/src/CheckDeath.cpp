#include "stdafx.h"
#include "CheckDeath.h"

#include "Script_EnemyManager.h"
#include "Script_LiveObject.h"

#include "AnimatorMotion.h"
#include "AnimatorController.h"
#include "Timer.h"


CheckDeath::CheckDeath(Object* object, std::function<void()> callback)
	: BT::ActionNode(callback)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mLiveObject = object->GetComponent<Script_LiveObject>();
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.DeathAnimName)->AddEndCallback(std::bind(&CheckDeath::DeathEndCallback, this));
	mRemoveTime = 4.f;
}

BT::NodeState CheckDeath::Evaluate()
{
	if (!mLiveObject->IsDead())
		return BT::NodeState::Failure;

	mEnemyMgr->mState = EnemyState::Death;

	mAccTime += DeltaTime();

	mEnemyMgr->RemoveAllAnimation();
	mEnemyMgr->mController->SetValue("Death", true);

	ExecuteCallback();

	if (mAccTime >= mRemoveTime) {
		mObject->mObjectCB.HitRimFactor = 0.7f;
		mObject->Destroy();
	}

	return BT::NodeState::Success;
}

void CheckDeath::DeathEndCallback()
{
	mEnemyMgr->mController->GetCrntMotion()->SetSpeed(0.f);
}
