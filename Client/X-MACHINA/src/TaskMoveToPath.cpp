#include "stdafx.h"
#include "TaskMoveToPath.h"

#include "Script_EnemyManager.h"
#include "AnimatorController.h"
#include "Timer.h"


TaskMoveToPath::TaskMoveToPath(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mMoveSpeed = mEnemyMgr->mMoveSpeed;
	mReturnSpeed = 1.7f * mMoveSpeed;
	mPath = &mEnemyMgr->mPath;
}


BT::NodeState TaskMoveToPath::Evaluate()
{
	if (mPath->empty())
		return BT::NodeState::Failure;

	mEnemyMgr->mController->SetValue("Walk", true);

	// 다음 경로까지의 벡터
	Vec3 nextPos = (mPath->top() - mObject->GetPosition()).xz();

	// 현재 복귀 상태라면 스피드를 올린다.
	const float speed = mEnemyMgr->mController->GetParam("Return")->val.b ? mReturnSpeed : mMoveSpeed;

	// 다음 경로를 향해 이동 및 회전
	mObject->RotateTargetAxisY(mPath->top(), mEnemyMgr->mRotationSpeed * DeltaTime());
	mObject->Translate(XMVector3Normalize(nextPos), speed * DeltaTime());

	// 다음 경로에 도착 시 해당 경로 삭제
	const float kMinDistance = 0.1f;
	if (nextPos.Length() < kMinDistance)
		mPath->pop();

	return BT::NodeState::Success;
}
