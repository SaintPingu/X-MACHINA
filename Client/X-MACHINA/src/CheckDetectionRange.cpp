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
	mPlayer = engine->GetPlayer();
}


BT::NodeState CheckDetectionRange::Evaluate()
{
	sptr<Object> target = GetData("target");

	if (!target) {
		mRoot->SetData("target", mPlayer);
		target = mPlayer;
	}

	// Player�� ���� ���� ��� ���Ŀ� ����
	if ((mObject->GetPosition() - target->GetPosition()).Length() < mEnemyMgr->mDetectionRange) {

		// Ÿ���� ������ ��ġ�� �ִٸ� ��� ������ �Ѵ�.
		if (scene->GetTileFromPos(target->GetPosition()) == Tile::Static)
			return BT::NodeState::Failure;

		mEnemyMgr->mController->SetValue("Walk", true);
		return BT::NodeState::Success;
	}

	return BT::NodeState::Failure;
}
