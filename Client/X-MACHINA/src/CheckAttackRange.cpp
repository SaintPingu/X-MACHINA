#include "stdafx.h"
#include "CheckAttackRange.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Object.h"

#include "AnimatorMotion.h"
#include "AnimatorController.h"


CheckAttackRange::CheckAttackRange(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mAttackName)->AddEndCallback(std::bind(&CheckAttackRange::AttackEndCallback, this));
}

BT::NodeState CheckAttackRange::Evaluate()
{
	if (!mEnemyMgr->mTarget) {
		return BT::NodeState::Failure;
	}

	if (mEnemyMgr->mState == EnemyState::Attack) {
		return BT::NodeState::Success;
	}

	if ((mObject->GetPosition() - mEnemyMgr->mTarget->GetPosition()).Length() < mEnemyMgr->mAttackRange) {
		const Vec3 toTargetDir = Vector3::Normalized(mEnemyMgr->mTarget->GetPosition() - mObject->GetPosition());
		const float angle = Vector3::Angle(mObject->GetLook(), toTargetDir);
		if (angle < 20.f) {
			mEnemyMgr->mState = EnemyState::Attack;
			mEnemyMgr->RemoveAllAnimation();
			mEnemyMgr->mController->SetValue("Attack", true);

			return BT::NodeState::Success;
		}
	}

	return BT::NodeState::Failure;
}

void CheckAttackRange::AttackEndCallback()
{
	mEnemyMgr->mController->SetValue("Attack", false);
	mEnemyMgr->mState = EnemyState::Idle;
}
