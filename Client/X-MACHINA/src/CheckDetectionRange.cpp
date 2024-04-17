#include "stdafx.h"
#include "CheckDetectionRange.h"

#include "Script_EnemyManager.h"

#include "X-Engine.h"
#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"


CheckDetectionRange::CheckDetectionRange(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mPlayer = Engine::I->GetPlayer();
}


BT::NodeState CheckDetectionRange::Evaluate()
{
	sptr<Object> target = GetData("target");

	if (!target) {
		mRoot->SetData("target", mPlayer);
		target = mPlayer;
	}

	// ��� ��ã�Ⱑ �������̰ų� ���� ���� ���� ���� ��� ���� ���� ����
	if ((mObject->GetPosition() - target->GetPosition()).Length() < mEnemyMgr->mDetectionRange) {

		// Ÿ���� ������ ��ġ�� �ִٸ� ��� ������ �Ѵ�.
		if (Scene::I->GetTileFromPos(target->GetPosition()) == Tile::Static)
			return BT::NodeState::Failure;

		mEnemyMgr->mController->SetValue("Walk", true);
		mEnemyMgr->mController->SetValue("Return", false);

		return BT::NodeState::Success;
	}

	return BT::NodeState::Failure;
}
