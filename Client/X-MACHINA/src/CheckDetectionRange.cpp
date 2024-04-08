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

	// Player가 여러 명일 경우 추후에 수정
	if ((mObject->GetPosition() - target->GetPosition()).Length() < mEnemyMgr->mDetectionRange) {

		// 타겟이 정적인 위치에 있다면 계속 정찰만 한다.
		if (scene->GetTileFromPos(target->GetPosition()) == Tile::Static)
			return BT::NodeState::Failure;

		mEnemyMgr->mController->SetValue("Walk", true);
		return BT::NodeState::Success;
	}

	return BT::NodeState::Failure;
}
