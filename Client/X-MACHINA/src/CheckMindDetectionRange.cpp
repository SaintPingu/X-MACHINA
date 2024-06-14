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
	// 가장 가까운 적을 타겟으로 설정
	if (!SetTargetNearestEnemy()) {
		mEnemyMgr->mState = EnemyState::Walk;
		mEnemyMgr->mController->SetValue("Walk", true);
		return BT::NodeState::Running;
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
			// 해당 오브젝트가 자신이면 넘어감
			if (object == mObject) {
				continue;
			}

			// 해당 오브젝트가 죽었다면 넘어감
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
