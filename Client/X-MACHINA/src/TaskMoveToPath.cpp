#include "stdafx.h"
#include "TaskMoveToPath.h"

#include "Script_EnemyManager.h"

#include "AnimatorMotion.h"
#include "AnimatorController.h"
#include "Timer.h"


TaskMoveToPath::TaskMoveToPath(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
	mMoveSpeed = mEnemyMgr->mStat.MoveSpeed;
	mReturnSpeed = 1.7f * mMoveSpeed;
	mPath = &mEnemyMgr->mPath;

	mReturnParam = mEnemyMgr->mController->GetParamRef("Return");
	mWalkMotion = mEnemyMgr->mController->FindMotionByName("WalkBackwards");
}


BT::NodeState TaskMoveToPath::Evaluate()
{
	if (mPath->empty())
		return BT::NodeState::Failure;

	mEnemyMgr->mController->SetValue("Walk", true);

	// 다음 경로까지의 벡터
	Vec3 nextPos = (mPath->top() - mObject->GetPosition()).xz();

	// 현재 복귀 상태라면 스피드를 올린다.
	float speed{};
	if (mReturnParam->val.b) {
		speed = mReturnSpeed;
		//mWalkMotion->SetSpeed(1.7f);
	}
	else {
		speed = mMoveSpeed;
		//mWalkMotion->SetSpeed(1.f);
	}

	// 다음 경로를 향해 이동 및 회전
	mObject->RotateTargetAxisY(mPath->top(), mEnemyMgr->mStat.MoveSpeed);
	mObject->Translate(XMVector3Normalize(nextPos), speed * DeltaTime());

	// 다음 경로에 도착 시 해당 경로 삭제
	const float kMinDistance = 0.1f;
	if (nextPos.Length() < kMinDistance)
		mPath->pop();

	return BT::NodeState::Success;
}
