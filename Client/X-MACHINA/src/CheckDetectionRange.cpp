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

	// 경로 길찾기가 실행중이거나 감지 범위 내에 들어온 경우 다음 노드로 진행
	if ((mObject->GetPosition() - mEnemyMgr->mTarget->GetPosition()).Length() < mEnemyMgr->mDetectionRange) {
		mEnemyMgr->mState = EnemyState::Walk;
		mEnemyMgr->mController->SetValue("Walk", true);
		return BT::NodeState::Success;
	}

	return BT::NodeState::Failure;
}
