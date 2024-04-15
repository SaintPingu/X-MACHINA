#include "stdafx.h"
#include "TaskGetHit.h"

#include "Script_EnemyManager.h"
#include "Script_LiveObject.h"
#include "AnimatorController.h"

TaskGetHit::TaskGetHit(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mLiveObject = object->GetComponent<Script_LiveObject>();
	mPrevHp = mLiveObject->GetCrntHp();
	mKnockBack = 0.1f;
}


BT::NodeState TaskGetHit::Evaluate()
{
	sptr<Object> target = GetData("target");

	if (!target) {
		return BT::NodeState::Failure;
	}

	const float crntHp = mLiveObject->GetCrntHp();

	if (mPrevHp != crntHp) {
		mEnemyMgr->mController->SetValue("GetHit", true);
		mEnemyMgr->mController->SetValue("Walk", false);
		mEnemyMgr->mController->SetValue("Attack", false);

		mPrevHp = crntHp;

		mObject->Translate(target->GetLook(), mKnockBack);

		return BT::NodeState::Success;
	}

	return BT::NodeState::Failure;
}
