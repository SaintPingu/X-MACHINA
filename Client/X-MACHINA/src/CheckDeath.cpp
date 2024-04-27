#include "stdafx.h"
#include "CheckDeath.h"

#include "Script_EnemyManager.h"
#include "Script_LiveObject.h"

#include "AnimatorMotion.h"
#include "AnimatorController.h"
#include "Timer.h"


CheckDeath::CheckDeath(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mLiveObject = object->GetComponent<Script_LiveObject>();

	mEnemyMgr->mController->FindMotionByName("Death")->AddEndCallback(std::bind(&CheckDeath::DeathCallback, this));
}


BT::NodeState CheckDeath::Evaluate()
{
	if (!mLiveObject->GetIsDead())
		return BT::NodeState::Failure;
	
	mAccTime += DeltaTime();
	mEnemyMgr->mController->SetValue("Death", true);

	if (mAccTime >= 2.f) {
		mObject->Destroy();
	}

	// 실행안됨.
	if (mAccTime >= 4.f) {
		// 임시로 다시 태어나도록 설정
		mEnemyMgr->mController->SetValue("Death", false);
		mLiveObject->Resurrect();
		mAccTime = 0.f;
		mObject->mObjectCB.DeathElapsed = 0.f;

		return BT::NodeState::Success;
	}

	return BT::NodeState::Success;
}

void CheckDeath::DeathCallback()
{
	mEnemyMgr->mController->GetCrntMotion()->SetSpeed(0.f);
}
