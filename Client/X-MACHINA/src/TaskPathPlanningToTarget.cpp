#include "stdafx.h"
#include "TaskPathPlanningToTarget.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "BattleScene.h"
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
	if (!mEnemyMgr->mPathTarget) {
		return BT::NodeState::Failure;
	}

	// ��ΰ� ����ٸ� ��� �� Ž��
	if (mPath->empty()){
		mEnemyMgr->mController->SetValue("Return", false);

		// ���� ������ ������ ��ġ ���� Ÿ�� ���� �ε����� ��ȯ
		Pos start = BattleScene::I->GetTileUniqueIndexFromPos(mObject->GetPosition());

		Pos dest = BattleScene::I->GetTileUniqueIndexFromPos(mEnemyMgr->mPathTarget->GetPosition());

		// ��� ��ȹ�� �����ߴٸ� Failure�� ȣ���Ͽ� ���� ���� �Ѿ
		if (base::PathPlanningAStar(start, dest)) {
			return BT::NodeState::Success;
		}
	}

	return BT::NodeState::Failure;
}
