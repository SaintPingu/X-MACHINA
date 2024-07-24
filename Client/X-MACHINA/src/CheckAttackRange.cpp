#include "stdafx.h"
#include "CheckAttackRange.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Object.h"

#include "AnimatorMotion.h"
#include "AnimatorController.h"

#include "Script_AbilityHolder.h"
#include "CloakingAbility.h"

CheckAttackRange::CheckAttackRange(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}

BT::NodeState CheckAttackRange::Evaluate()
{
	if (!mEnemyMgr->mTarget) {
		return BT::NodeState::Failure;
	}

	if (mEnemyMgr->mState == EnemyState::Attack) {
		return BT::NodeState::Success;
	}

	const auto& abilitys = mEnemyMgr->mTarget->GetComponents<Script_AbilityHolder>();
	for (const auto& ability : abilitys) {
		if (ability->GetAbilityName() == "Cloaking" && ability->GetAbilityState() == AbilityState::Active) {
			mEnemyMgr->mTarget = nullptr;
			return BT::NodeState::Failure;
		}
	}

	constexpr float minDistance = 1.f;
	const float distance = (mObject->GetPosition() - mEnemyMgr->mTarget->GetPosition()).Length();
	if (distance < mEnemyMgr->mStat.AttackRange) {
		const Vec3 toTargetDir = Vector3::Normalized(mEnemyMgr->mTarget->GetPosition() - mObject->GetPosition());
		const float angle = Vector3::Angle(mObject->GetLook(), toTargetDir);
		if (minDistance < 1.f || angle < 80.f) {
			mEnemyMgr->mState = EnemyState::Attack;
			return BT::NodeState::Success;
		}
	}

	return BT::NodeState::Failure;
}
