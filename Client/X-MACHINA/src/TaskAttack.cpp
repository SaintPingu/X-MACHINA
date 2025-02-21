#include "stdafx.h"
#include "TaskAttack.h"

#include "Script_EnemyManager.h"
#include "Script_LiveObject.h"

#include "Timer.h"
#include "BattleScene.h"
#include "Object.h"

#include "AnimatorMotion.h"
#include "AnimatorController.h"


TaskAttack::TaskAttack(Object* object, std::function<void()> callback)
	: BT::ActionNode(callback)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mLiveObject = object->GetComponent<Script_LiveObject>();
}

BT::NodeState TaskAttack::Evaluate()
{
	mObject->RotateTargetAxisY(mEnemyMgr->mTarget->GetPosition(), mEnemyMgr->mStat.AttackRotationSpeed);
	mLiveObject->UpdatePrevHP();

	ExecuteCallback();

	return BT::NodeState::Success;
}