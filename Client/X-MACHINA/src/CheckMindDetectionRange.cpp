#include "stdafx.h"
#include "CheckMindDetectionRange.h"

#include "Script_EnemyManager.h"
#include "Script_LiveObject.h"

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
	// ���� ����� ���� Ÿ������ ����
	if (!SetTargetNearestEnemy()) {
		mEnemyMgr->mState = EnemyState::Walk;
		mEnemyMgr->mController->SetValue("Walk", true);
		return BT::NodeState::Running;
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

bool CheckMindDetectionRange::SetTargetNearestEnemy()
{
	if (mEnemyMgr->mTarget) {
		return true;
	}

	bool isSetTarget = false;
	float minDistance = FLT_MAX;
	const std::vector<sptr<Grid>>& grids = Scene::I->GetNeighborGrids(Scene::I->GetGridIndexFromPos(mObject->GetPosition()), true);
	for (const auto& grid : grids) {
		for (const auto& object : grid->GetObjectsFromTag(ObjectTag::Enemy)) {
			// �ش� ������Ʈ�� �ڽ��̸� �Ѿ
			if (object == mObject) {
				continue;
			}

			// �ش� ������Ʈ�� �׾��ٸ� �Ѿ
			if (object->GetComponent<Script_LiveObject>()->IsDead()) {
				continue;
			}

			float distance = Vec3::Distance(mObject->GetPosition(), object->GetPosition());
			if (distance < minDistance && distance < mEnemyMgr->mStat.DetectionRange) {
				minDistance = distance;
				mEnemyMgr->mTarget = object;
				isSetTarget = true;
			}
		}
	}

	return isSetTarget;
}
