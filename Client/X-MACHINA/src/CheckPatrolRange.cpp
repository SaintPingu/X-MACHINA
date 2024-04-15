#include "stdafx.h"
#include "CheckPatrolRange.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Object.h"
#include "AnimatorController.h"


CheckPatrolRange::CheckPatrolRange(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();

	mSpawnPos = object->GetPosition();
}


BT::NodeState CheckPatrolRange::Evaluate()
{
	float dis = Vec3::Distance(mSpawnPos, mObject->GetPosition().xz());

	mEnemyMgr->mController->SetValue("Walk", true);
	mEnemyMgr->mController->SetValue("Return", true);

	if (dis < 10.f) {
		mEnemyMgr->mController->SetValue("Return", false);
		return BT::NodeState::Running;
	}

	return BT::NodeState::Failure;
}
