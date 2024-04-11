#include "stdafx.h"
#include "TaskReturnToSpawnAStar.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"
#include "MeshRenderer.h"
#include "Grid.h"


TaskReturnToSpawnAStar::TaskReturnToSpawnAStar(Object* object) : TaskPathPlanningAStar(object)
{
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();

	mSpawnPos = object->GetPosition();
}

BT::NodeState TaskReturnToSpawnAStar::Evaluate()
{
	// ��ΰ� ����ٸ� ��� �� Ž��
	if (mEnemyMgr->mPath.empty()) {
		// ���� ������ ������ ��ġ ���� Ÿ�� ���� �ε����� ��ȯ
		Pos start = scene->GetTileUniqueIndexFromPos(mObject->GetPosition());
		Pos dest = scene->GetTileUniqueIndexFromPos(mSpawnPos);

		// ��� ��ȹ�� �����ߴٸ� Failure�� ȣ���Ͽ� ���� ���� �Ѿ
		if (base::PathPlanningAStar(start, dest) == false)
			return BT::NodeState::Failure;
	}

	// ��η� �̵�
	base::MoveToPath();
	return BT::NodeState::Success;
}
