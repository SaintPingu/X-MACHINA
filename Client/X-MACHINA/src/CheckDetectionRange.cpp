#include "stdafx.h"
#include "CheckDetectionRange.h"

#include "GameFramework.h"
#include "Timer.h"
#include "BattleScene.h"
#include "Object.h"
#include "AnimatorController.h"

#include "Script_Enemy.h"
#include "Script_EnemyManager.h"
#include "Script_Ability_Cloaking.h"

CheckDetectionRange::CheckDetectionRange(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mTarget = GameFramework::I->GetPlayer();
}


BT::NodeState CheckDetectionRange::Evaluate()
{
	if (!mEnemyMgr->mTarget) { 
		mEnemyMgr->mTarget = mTarget;
	}

	if (!mEnemyMgr->mPath.empty()) {
		return BT::NodeState::Failure;
	}

	const auto& ability = mEnemyMgr->mTarget->GetComponent<Script_Ability_Cloaking>();
	if (ability && ability->GetState() == Script_Ability::State::Active) {
		mEnemyMgr->mTarget = nullptr;
		return BT::NodeState::Failure;
	}

	// ��� ��ã�Ⱑ �������̰ų� ���� ���� ���� ���� ��� ���� ���� ����
	if ((mObject->GetPosition() - mEnemyMgr->mTarget->GetPosition()).Length() < mEnemyMgr->mStat.DetectionRange) {
		mEnemyMgr->mState = EnemyState::Walk;
		mEnemyMgr->mController->SetValue("Walk", true);
		return BT::NodeState::Success;
	}
	else {
		mEnemyMgr->mTarget = nullptr;
	}

	return BT::NodeState::Failure;
}
