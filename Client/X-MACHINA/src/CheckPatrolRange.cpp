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
	// ���� ���� ��� CheckPatrolRange�� Failure�� �Ǿ�� �Ѵ�.
	mEnemyMgr->mController->SetValue("Walk", true);
	mEnemyMgr->mController->SetValue("Return", true);

	// ���� ����Ʈ ���� �߽����κ��� ���� �ָ� ������ ����Ʈ���� �Ÿ��� ����
	constexpr float adjRange = 0.2f;
	float dis = Vec3::Distance(mBaryCenter, mObject->GetPosition().xz());
	if (dis <= mPatrolRange + adjRange) {
		mEnemyMgr->mController->SetValue("Return", false);
		return BT::NodeState::Running;
	}

	return BT::NodeState::Failure;
}
