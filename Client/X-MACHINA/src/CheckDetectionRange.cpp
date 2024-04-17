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

	// 경로 길찾기가 실행중이거나 감지 범위 내에 들어온 경우 다음 노드로 진행
	if ((mObject->GetPosition() - target->GetPosition()).Length() < mEnemyMgr->mDetectionRange) {

		// 타겟이 정적인 위치에 있다면 계속 정찰만 한다.
		if (Scene::I->GetTileFromPos(target->GetPosition()) == Tile::Static)
			return BT::NodeState::Failure;

		mEnemyMgr->mController->SetValue("Walk", true);
		mEnemyMgr->mController->SetValue("Return", false);

		return BT::NodeState::Success;
	}

	return BT::NodeState::Failure;
}
