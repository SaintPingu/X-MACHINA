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

	if (dis < 0.1f) {
		return BT::NodeState::Running;
	}

	return BT::NodeState::Failure;
}
