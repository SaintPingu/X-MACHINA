#include "stdafx.h"
#include "CheckDetectionRange.h"

#include "Script_EnemyManager.h"

#include "GameFramework.h"
#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"


CheckDetectionRange::CheckDetectionRange(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mPlayer = GameFramework::I->GetPlayer();
}


BT::NodeState CheckDetectionRange::Evaluate()
{
	if (!mEnemyMgr->mTarget) {
		mEnemyMgr->mTarget = mPlayer;
	}

	// ��� ��ã�Ⱑ �������̰ų� ���� ���� ���� ���� ��� ���� ���� ����
	if ((mObject->GetPosition() - mEnemyMgr->mTarget->GetPosition()).Length() < mEnemyMgr->mDetectionRange) {

		// Ÿ���� ������ ��ġ�� �ִٸ� ��� ������ �Ѵ�.
		if (Scene::I->GetTileFromPos(mEnemyMgr->mTarget->GetPosition()) == Tile::Static)
			return BT::NodeState::Failure;

		mEnemyMgr->mController->SetValue("Walk", true);
		mEnemyMgr->mController->SetValue("Return", false);

		return BT::NodeState::Success;
	}

	return BT::NodeState::Failure;
}
