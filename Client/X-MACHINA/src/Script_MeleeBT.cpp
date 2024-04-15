#include "stdafx.h"
#include "Script_MeleeBT.h"

#include "CheckDetectionRange.h"
#include "CheckAttackRange.h"
#include "CheckPatrolRange.h"
#include "TaskPatrol.h"
#include "TaskMoveToTarget.h"
#include "TaskAttack.h"
#include "TaskPathPlanningToTarget.h"
#include "TaskPathPlanningToSpawn.h"
#include "TaskMoveToPath.h"
#include "Wait.h"

BT::Node* Script_MeleeBT::SetupTree()
{
	// 오브젝트 스폰 위치 중심 (5, 5) 만큼 정찰 위치 설정
	std::vector<Vec3> wayPoints(1);
	wayPoints[0] = mObject->GetLocalPosition() + Vec3(0.f, 0.f, 0.f);
	//wayPoints[0] = mObject->GetLocalPosition() + Vec3(5.f, 0.f, 5.f);
	//wayPoints[1] = mObject->GetLocalPosition() + Vec3(5.f, 0.f, -5.f);
	//wayPoints[2] = mObject->GetLocalPosition() + Vec3(-5.f, 0.f, -5.f);
	//wayPoints[3] = mObject->GetLocalPosition() + Vec3(-5.f, 0.f, 5.f);
	
	// 행동 트리 설정
	BT::Node* root = new BT::Selector{ std::vector<BT::Node*>{
		new BT::Sequence{ std::vector<BT::Node*>{
			new CheckAttackRange(mObject),
			new TaskAttack(mObject),
			new Wait(1.f) }},
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
				new CheckPatrolRange(mObject),
				new TaskPatrol(mObject, std::move(wayPoints)),
				}},
				new TaskPathPlanningToSpawn(mObject)
			}}
	}};

	return root;
}
