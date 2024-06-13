#include "stdafx.h"
#include "CheckMindDetectionRange.h"

#include "Script_EnemyManager.h"

#include "GameFramework.h"
#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"


CheckMindDetectionRange::CheckMindDetectionRange(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}


BT::NodeState CheckMindDetectionRange::Evaluate()
{
	if (!mEnemyMgr->mTarget) {
		float minDistance = FLT_MAX;
		const std::vector<sptr<Grid>>& grids = Scene::I->GetNeighborGrids(Scene::I->GetGridIndexFromPos(mObject->GetPosition()), true);
		for (const auto& grid : grids) {
			for (const auto& object : grid->GetObjectsFromTag(ObjectTag::Enemy)) {
				if (object == mObject) {
					continue;
				}

				float distance = Vec3::Distance(mObject->GetPosition(), object->GetPosition());
				if (distance < minDistance) {
					minDistance = distance;
					mEnemyMgr->mTarget = object;
				}
			}
		}
	}

	if (mEnemyMgr->mTarget->GetTag() != ObjectTag::Enemy) {
		mEnemyMgr->mTarget = nullptr;
		return BT::NodeState::Failure;
	}

	// ��� ��ã�Ⱑ �������̰ų� ���� ���� ���� ���� ��� ���� ���� ����
	if ((mObject->GetPosition() - mEnemyMgr->mTarget->GetPosition()).Length() < mEnemyMgr->mStat.DetectionRange) {
		mEnemyMgr->mState = EnemyState::Walk;
		mEnemyMgr->mController->SetValue("Walk", true);
		return BT::NodeState::Success;
	}

	return BT::NodeState::Failure;
}
