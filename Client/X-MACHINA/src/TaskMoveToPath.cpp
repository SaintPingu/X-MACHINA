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

	// ���� ��α����� ����
	Vec3 nextPos = (mPath->top() - mObject->GetPosition()).xz();

	// ���� ���� ���¶�� ���ǵ带 �ø���.
	float speed{};
	if (mReturnParam->val.b) {
		speed = mReturnSpeed;
		//mWalkMotion->SetSpeed(1.7f);
	}
	else {
		speed = mMoveSpeed;
		//mWalkMotion->SetSpeed(1.f);
	}

	// ���� ��θ� ���� �̵� �� ȸ��
	mObject->RotateTargetAxisY(mPath->top(), mEnemyMgr->mStat.MoveSpeed);
	mObject->Translate(XMVector3Normalize(nextPos), speed * DeltaTime());

	// ���� ��ο� ���� �� �ش� ��� ����
	const float kMinDistance = 0.1f;
	if (nextPos.Length() < kMinDistance)
		mPath->pop();

	return BT::NodeState::Success;
}
