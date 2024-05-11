#include "stdafx.h"
#include "TaskGetHit.h"
#include "Timer.h"

#include "Script_EnemyManager.h"
#include "Script_LiveObject.h"
#include "AnimatorMotion.h"
#include "AnimatorController.h"

TaskGetHit::TaskGetHit(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mLiveObject = object->GetComponent<Script_LiveObject>();
	mPrevHp = mLiveObject->GetCrntHp();
	mKnockBack = 0.05f;
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mGetHitName)->AddEndCallback(std::bind(&TaskGetHit::GetHitEndCallback, this));
}

BT::NodeState TaskGetHit::Evaluate()
{
	if (!mEnemyMgr->mTarget) {
			return BT::NodeState::Failure;
	}

	const float crntHp = mLiveObject->GetCrntHp();
	if (!mLiveObject->UpdatePrevHP()) {
		mEnemyMgr->mState = EnemyState::GetHit;
		mEnemyMgr->mController->SetValue("GetHit", true);

		mObject->Translate(mEnemyMgr->mTarget->GetLook(), mKnockBack);
	}

	if (mEnemyMgr->mController->GetParamRef("GetHit")->val.b == false)
		return BT::NodeState::Failure;

	return BT::NodeState::Success;
}

void TaskGetHit::GetHitEndCallback()
{
	mEnemyMgr->mController->SetValue("GetHit", false);
}
