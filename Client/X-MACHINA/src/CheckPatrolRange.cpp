#include "stdafx.h"
#include "CheckPatrolRange.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Object.h"
#include "AnimatorController.h"


CheckPatrolRange::CheckPatrolRange(Object* object, const Vec3& baryCenter, float patrolRange)
	:
	mBaryCenter(baryCenter),
	mPatrolRange(patrolRange)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}


BT::NodeState CheckPatrolRange::Evaluate()
{
	// 복귀 중인 경우 CheckPatrolRange는 Failure가 되어야 한다.
	mEnemyMgr->mController->SetValue("Walk", true);
	mEnemyMgr->mController->SetValue("Return", true);

	// 웨이 포인트 무게 중심으로부터 가장 멀리 떨어진 포인트와의 거리가 범위
	constexpr float adjRange = 0.2f;
	float dis = Vec3::Distance(mBaryCenter, mObject->GetPosition().xz());
	if (dis <= mPatrolRange + adjRange) {
		mEnemyMgr->mController->SetValue("Return", false);
		return BT::NodeState::Running;
	}

	return BT::NodeState::Failure;
}
