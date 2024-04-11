#include "stdafx.h"
#include "TaskMoveToTargetAStar.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"
#include "MeshRenderer.h"
#include "Grid.h"


TaskMoveToTargetAStar::TaskMoveToTargetAStar(Object* object) : TaskPathPlanningAStar(object)
{
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}

BT::NodeState TaskMoveToTargetAStar::Evaluate()
{
	sptr<Object> target = GetData("target");

	// 경로가 비었다면 경로 재 탐색
	if (mPath->empty()){
		// 시작 지점과 목적지 위치 값을 타일 고유 인덱스로 변환
		Pos start = scene->GetTileUniqueIndexFromPos(mObject->GetPosition());
		Pos dest = scene->GetTileUniqueIndexFromPos(target->GetPosition());

		// 경로 계획에 실패했다면 Failure를 호출하여 다음 노드로 넘어감
		if (base::PathPlanningAStar(start, dest) == false)
			return BT::NodeState::Failure;
	}

	// 경로로 이동
	base::MoveToPath();
	return BT::NodeState::Success;
}
