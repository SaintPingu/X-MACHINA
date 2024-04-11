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

	// ��ΰ� ����ٸ� ��� �� Ž��
	if (mPath->empty()){
		// ���� ������ ������ ��ġ ���� Ÿ�� ���� �ε����� ��ȯ
		Pos start = scene->GetTileUniqueIndexFromPos(mObject->GetPosition());
		Pos dest = scene->GetTileUniqueIndexFromPos(target->GetPosition());

		// ��� ��ȹ�� �����ߴٸ� Failure�� ȣ���Ͽ� ���� ���� �Ѿ
		if (base::PathPlanningAStar(start, dest) == false)
			return BT::NodeState::Failure;
	}

	// ��η� �̵�
	base::MoveToPath();
	return BT::NodeState::Success;
}
