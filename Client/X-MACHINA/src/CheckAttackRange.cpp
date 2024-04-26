#include "stdafx.h"
#include "CheckAttackRange.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Object.h"
#include "AnimatorController.h"


CheckAttackRange::CheckAttackRange(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}


BT::NodeState CheckAttackRange::Evaluate()
{
	if (!mEnemyMgr->mTarget) {
		return BT::NodeState::Failure;
	}

	// TODO : 공격 범위에 들어온 이후 공격 애니메이션 계속 진행(공격 애니메이션의 속도와 쿨타임을 맞출 것)
	if ((mObject->GetPosition() - mEnemyMgr->mTarget->GetPosition()).Length() < mEnemyMgr->mAttackRange) {

		mEnemyMgr->mController->SetValue("Attack", true);
		mEnemyMgr->mController->SetValue("Walk", false);

		return BT::NodeState::Success;
	}


	return BT::NodeState::Failure;
}
