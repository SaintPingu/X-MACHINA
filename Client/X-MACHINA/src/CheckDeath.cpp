#include "stdafx.h"
#include "CheckDeath.h"

#include "Script_EnemyManager.h"
#include "Script_LiveObject.h"
#include "AnimatorController.h"
#include "Timer.h"


CheckDeath::CheckDeath(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mLiveObject = object->GetComponent<Script_LiveObject>();
}


BT::NodeState CheckDeath::Evaluate()
{
	if (!mLiveObject->GetIsDead())
		return BT::NodeState::Failure;
	
	mAccTime += DeltaTime();
	mObject->mObjectCB.DeathElapsed += DeltaTime();
	mEnemyMgr->mController->SetValue("Death", true);

	if (mAccTime >= 1.f) {
		// 임시로 다시 태어나도록 설정
		mEnemyMgr->mController->SetValue("Death", false);
		mLiveObject->Resurrect();
		mAccTime = 0.f;
		mObject->mObjectCB.DeathElapsed = 0.f;

		//mObject->OnDestroy();
		return BT::NodeState::Success;
	}

	return BT::NodeState::Success;
}
