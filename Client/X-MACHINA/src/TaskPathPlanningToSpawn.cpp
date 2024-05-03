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
		mEnemyMgr->mController->SetValue("Return", true);

		// ���� ������ ������ ��ġ ���� Ÿ�� ���� �ε����� ��ȯ
		Pos start = Scene::I->GetTileUniqueIndexFromPos(mObject->GetPosition());
		Pos dest = Scene::I->GetTileUniqueIndexFromPos(mSpawnPos);

		// ��� ��ȹ�� �����ߴٸ� Failure�� ȣ���Ͽ� ���� ���� �Ѿ
		if (base::PathPlanningAStar(start, dest)) {
			return BT::NodeState::Success;
		}
	}

	return BT::NodeState::Failure;
}
