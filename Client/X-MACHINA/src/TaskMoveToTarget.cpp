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
	Vec3 toTarget =  mObject->GetPosition() - target->GetPosition();

	// 타겟으로부터 오브젝트로 광선을 쏜다.
	Ray r{ target->GetPosition(), XMVector3Normalize(toTarget)};

	// 해당 광선에 맞은 다른 Static 오브젝트의 거리가 타겟까지의 거리보다 가까운 경우 벽에 막혀있는 경우이다.
	if (scene->CheckCollisionsRay(r) < toTarget.Length()) {
		return BT::NodeState::Failure;
	}

	// 오브젝트로부터 타겟까지의 벡터
	const float kMinDistance = 0.1f;
	
	// 한 번이라도 해당 노드가 실행됐다면 기존 길찾기 경로를 삭제 하기 위한 플래그
	mEnemyMgr->mIsMoveToPath = false;

	// 타겟에 도착하지 않았을 경우에만 이동
	if (toTarget.Length() > kMinDistance) {
		mObject->RotateTargetAxisY(target->GetPosition(), mEnemyMgr->mRotationSpeed * DeltaTime());
		mObject->Translate(mObject->GetLook(), mEnemyMgr->mMoveSpeed * DeltaTime());
	}

	return BT::NodeState::Success;
}
