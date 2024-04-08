#include "stdafx.h"
#include "TaskMoveToTarget.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"
#include "MeshRenderer.h"

TaskMoveToTarget::TaskMoveToTarget(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}

BT::NodeState TaskMoveToTarget::Evaluate()
{
	sptr<Object> target = GetData("target");

	// 오브젝트로부터 타겟까지의 벡터
	Vec3 toTarget = mObject->GetPosition() - target->GetPosition();

	// 오브젝트의 다음 이동 위치 타일이 막혀있는 경우 AStar를 실행시켜야 한다.
	Vec3 nextPos = mObject->GetLook() * mEnemyMgr->mMoveSpeed * DeltaTime() + mObject->GetPosition();

	// 다음 위치의 타일이 막혀 있거나 현재 길찾기 진행중이라면 진행하지 않는다.
	if (scene->GetTileFromPos(nextPos) == Tile::Static || mEnemyMgr->mIsMoveToPath) {
		return BT::NodeState::Failure;
	}

	// 오브젝트로부터 타겟까지의 벡터
	const float kMinDistance = 0.1f;

	// 타겟에 도착하지 않았을 경우에만 이동
	if (toTarget.Length() > kMinDistance) {
		mObject->RotateTargetAxisY(target->GetPosition(), mEnemyMgr->mRotationSpeed * DeltaTime());
		mObject->Translate(mObject->GetLook(), mEnemyMgr->mMoveSpeed * DeltaTime());
	}

	return BT::NodeState::Success;
}
