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

	// ���� ��α����� ����
	Vec3 nextPos = (mPath->top() - mObject->GetPosition()).xz();

	// ���� ���� ���¶�� ���ǵ带 �ø���.
	const float speed = mEnemyMgr->mController->GetParam("Return")->val.b ? mReturnSpeed : mMoveSpeed;

	// ���� ��θ� ���� �̵� �� ȸ��
	mObject->RotateTargetAxisY(mPath->top(), mEnemyMgr->mRotationSpeed * DeltaTime());
	mObject->Translate(XMVector3Normalize(nextPos), speed * DeltaTime());

	// ���� ��ο� ���� �� �ش� ��� ����
	const float kMinDistance = 0.1f;
	if (nextPos.Length() < kMinDistance)
		mPath->pop();

	return BT::NodeState::Success;
}
