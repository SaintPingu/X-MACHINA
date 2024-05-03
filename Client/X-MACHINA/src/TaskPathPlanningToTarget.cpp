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
	// ��ΰ� ����ٸ� ��� �� Ž��
	if (mPath->empty()){
		mEnemyMgr->mController->SetValue("Return", false);

		// ���� ������ ������ ��ġ ���� Ÿ�� ���� �ε����� ��ȯ
		Pos start = Scene::I->GetTileUniqueIndexFromPos(mObject->GetPosition());

		Pos dest = Scene::I->GetTileUniqueIndexFromPos(mEnemyMgr->mTarget->GetPosition());

		// ��� ��ȹ�� �����ߴٸ� Failure�� ȣ���Ͽ� ���� ���� �Ѿ
		if (base::PathPlanningAStar(start, dest)) {
			return BT::NodeState::Success;
		}
	}

	return BT::NodeState::Failure;
}
