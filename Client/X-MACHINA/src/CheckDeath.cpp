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

	mRemoveTime = 4.f;
}


BT::NodeState CheckDeath::Evaluate()
{
	if (!mLiveObject->GetIsDead())
		return BT::NodeState::Failure;
	
	mAccTime += DeltaTime();
	mEnemyMgr->mController->SetValue("Death", true);

	if (mAccTime >= mRemoveTime) {
		mObject->Destroy();
	}

	return BT::NodeState::Success;
}

void CheckDeath::DeathCallback()
{
	mEnemyMgr->mController->GetCrntMotion()->SetSpeed(0.f);
}
