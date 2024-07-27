#include "stdafx.h"
#include "CheckDetectionRange.h"

#include "GameFramework.h"
#include "Timer.h"
#include "BattleScene.h"
#include "Object.h"
#include "AnimatorController.h"

#include "Script_EnemyManager.h"
#include "Script_AbilityHolder.h"
#include "CloakingAbility.h"

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

	const auto& abilitys = mEnemyMgr->mTarget->GetComponents<Script_AbilityHolder>();
	for (const auto& ability : abilitys) {
		if (ability->GetAbilityName() == "Cloaking" && ability->GetAbilityState() == AbilityState::Active) {
			mEnemyMgr->mTarget = nullptr;
			return BT::NodeState::Failure;
		}
	}

	// 경로 길찾기가 실행중이거나 감지 범위 내에 들어온 경우 다음 노드로 진행
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
