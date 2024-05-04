#include "stdafx.h"
#include "Script_MeleeBT.h"

#include "Script_Enemy.h"

#include "CheckDetectionRange.h"
#include "CheckAttackRange.h"
#include "CheckPatrolRange.h"
#include "CheckDeath.h"
#include "TaskPatrol.h"
#include "TaskMoveToTarget.h"
#include "TaskAttack.h"
#include "TaskPathPlanningToTarget.h"
#include "TaskPathPlanningToSpawn.h"
#include "TaskMoveToPath.h"
#include "TaskGetHit.h"
#include "Wait.h"

BT::Node* Script_MeleeBT::SetupTree()
{
#pragma region WayPoint
	// ������Ʈ ���� ��ġ �߽� (5, 5) ��ŭ ���� ��ġ ����
	std::vector<Vec3> wayPoints(3);
	const float waySize = 5.f;
	wayPoints[0] = mObject->GetLocalPosition();
	wayPoints[1] = mObject->GetLocalPosition() + Vec3(waySize, 0.f, 0.f);
	wayPoints[2] = mObject->GetLocalPosition() + Vec3(waySize, 0.f, waySize);

	// ���� �߽� ��ġ ���
	const Vec3 baryCenter = std::accumulate(wayPoints.begin(), wayPoints.end(), Vec3{ 0.f, 0.f, 0.f }) / 3.f;

	// ���� �߽����κ��� ���� �� ���� ���
	float maxDis{};
	for (const auto& wayPoint : wayPoints)
		maxDis = max(maxDis, Vec3::Distance(baryCenter, wayPoint));
#pragma endregion

	auto& enemy = mObject->GetComponent<Script_Enemy>();

#pragma region BehaviorTree
	BT::Node* root = new BT::Selector{ std::vector<BT::Node*>{
		new CheckDeath(mObject, std::bind(&Script_Enemy::Death, enemy)),
		new BT::Sequence{ std::vector<BT::Node*>{
			new CheckAttackRange(mObject),
			new TaskAttack(mObject, std::bind(&Script_Enemy::Attack, enemy)),
			}},
		new TaskGetHit(mObject),
		new BT::Sequence{ std::vector<BT::Node*>{
			new CheckDetectionRange(mObject),
			new BT::Selector{ std::vector<BT::Node*>{
				new TaskMoveToTarget(mObject),
				new TaskPathPlanningToTarget(mObject),
				}},
			}},
		new TaskMoveToPath(mObject),
		new BT::Selector{ std::vector<BT::Node*>{
			new BT::Sequence{ std::vector<BT::Node*>{
				new CheckPatrolRange(mObject, baryCenter, maxDis),
				new TaskPatrol(mObject, std::move(wayPoints)),
				}},
				new TaskPathPlanningToSpawn(mObject)
			}}
	}};
#pragma endregion

	return root;
}
