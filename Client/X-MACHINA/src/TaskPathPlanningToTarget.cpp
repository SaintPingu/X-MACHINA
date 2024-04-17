#include "stdafx.h"
#include "TaskPathPlanningToTarget.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"
#include "MeshRenderer.h"
#include "Grid.h"


TaskPathPlanningToTarget::TaskPathPlanningToTarget(Object* object) : TaskPathPlanningAStar(object)
{
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}

BT::NodeState TaskPathPlanningToTarget::Evaluate()
{
	sptr<Object> target = GetData("target");

	// 경로가 비었다면 경로 재 탐색
	if (mPath->empty()){
		// 시작 지점과 목적지 위치 값을 타일 고유 인덱스로 변환
		Pos start = scene->GetTileUniqueIndexFromPos(mObject->GetPosition());
		Pos dest = scene->GetTileUniqueIndexFromPos(target->GetPosition());

		// 경로 계획에 실패했다면 Failure를 호출하여 다음 노드로 넘어감
		if (base::PathPlanningAStar(start, dest)) {
			return BT::NodeState::Success;
		}
	}

	return BT::NodeState::Failure;
}
