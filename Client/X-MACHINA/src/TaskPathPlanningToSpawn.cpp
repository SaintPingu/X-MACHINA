#include "stdafx.h"
#include "TaskPathPlanningToSpawn.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"
#include "MeshRenderer.h"
#include "Grid.h"


TaskPathPlanningToSpawn::TaskPathPlanningToSpawn(Object* object) : TaskPathPlanningAStar(object)
{
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();

	mSpawnPos = object->GetPosition();
}

BT::NodeState TaskPathPlanningToSpawn::Evaluate()
{
	// ��ΰ� ����ٸ� ��� �� Ž��
	if (mEnemyMgr->mPath.empty()) {
		// ���� ������ ������ ��ġ ���� Ÿ�� ���� �ε����� ��ȯ
		Pos start = scene->GetTileUniqueIndexFromPos(mObject->GetPosition());
		Pos dest = scene->GetTileUniqueIndexFromPos(mSpawnPos);

		// ��� ��ȹ�� �����ߴٸ� Failure�� ȣ���Ͽ� ���� ���� �Ѿ
		if (base::PathPlanningAStar(start, dest)) {
			mEnemyMgr->mController->SetValue("Return", true);
			return BT::NodeState::Success;
		}
	}

	return BT::NodeState::Failure;
}
