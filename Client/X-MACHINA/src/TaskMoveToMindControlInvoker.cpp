#include "stdafx.h"
#include "TaskMoveToMindControlInvoker.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"
#include "MeshRenderer.h"

TaskMoveToMindControlInvoker::TaskMoveToMindControlInvoker(Object* object, Object* invoker)
{
	mObject = object;
	mInvoker = invoker;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}

BT::NodeState TaskMoveToMindControlInvoker::Evaluate()
{
	Object* target = mEnemyMgr->mTarget;
	if (!target) {
		if (mInvoker) {
			target = mInvoker;
			mEnemyMgr->mPathTarget = mInvoker;
		}
		else {
			return BT::NodeState::Failure;
		}
	}

	// 허리 쪽부터 광선을 쏴야 맞는다.
	Vec3 objectAdjPos = mObject->GetPosition() + mObject->GetUp() * 0.5f;
	Vec3 targetAdjPos = target->GetPosition() + target->GetUp() * 0.5f;

	// 오브젝트로부터 타겟까지의 벡터
	Vec3 toTarget = targetAdjPos - objectAdjPos;

	// 타겟으로부터 오브젝트로 광선을 쏜다.
	Ray r{ objectAdjPos, XMVector3Normalize(toTarget)};

	// 타겟이 속한 모든 그리드를 검사해야 한다.
	GridObject* mGridTarget = dynamic_cast<GridObject*>(target);

	// 해당 광선에 맞은 다른 Static 오브젝트의 거리가 타겟까지의 거리보다 가까운 경우 벽에 막혀있는 경우이다.
	if (mGridTarget) {
		for (const auto& gridIndex : mGridTarget->GetGridIndices()) {
			if (Scene::I->CheckCollisionsRay(gridIndex, r) < toTarget.Length()) {
				return BT::NodeState::Failure;
			}
		}
	}

	// 한 번이라도 장애물 없이 직선 경로라면 길찾기 경로 초기화
	while (!mEnemyMgr->mPath.empty())
		mEnemyMgr->mPath.pop();

	// 오브젝트로부터 타겟까지의 벡터
	const float kMinDistance = 0.1f;
	
	// 타겟에 도착하지 않았을 경우에만 이동
	if (toTarget.Length() > kMinDistance) {
		mEnemyMgr->mController->SetValue("Return", false);

		mObject->RotateTargetAxisY(target->GetPosition(), mEnemyMgr->mStat.RotationSpeed);
		mObject->Translate(mObject->GetLook(), mEnemyMgr->mStat.MoveSpeed * DeltaTime());
	}

	return BT::NodeState::Success;
}
