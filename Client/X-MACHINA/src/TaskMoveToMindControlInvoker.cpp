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

	// �㸮 �ʺ��� ������ ���� �´´�.
	Vec3 objectAdjPos = mObject->GetPosition() + mObject->GetUp() * 0.5f;
	Vec3 targetAdjPos = target->GetPosition() + target->GetUp() * 0.5f;

	// ������Ʈ�κ��� Ÿ�ٱ����� ����
	Vec3 toTarget = targetAdjPos - objectAdjPos;

	// Ÿ�����κ��� ������Ʈ�� ������ ���.
	Ray r{ objectAdjPos, XMVector3Normalize(toTarget)};

	// Ÿ���� ���� ��� �׸��带 �˻��ؾ� �Ѵ�.
	GridObject* mGridTarget = dynamic_cast<GridObject*>(target);

	// �ش� ������ ���� �ٸ� Static ������Ʈ�� �Ÿ��� Ÿ�ٱ����� �Ÿ����� ����� ��� ���� �����ִ� ����̴�.
	if (mGridTarget) {
		for (const auto& gridIndex : mGridTarget->GetGridIndices()) {
			if (Scene::I->CheckCollisionsRay(gridIndex, r) < toTarget.Length()) {
				return BT::NodeState::Failure;
			}
		}
	}

	// �� ���̶� ��ֹ� ���� ���� ��ζ�� ��ã�� ��� �ʱ�ȭ
	while (!mEnemyMgr->mPath.empty())
		mEnemyMgr->mPath.pop();

	// ������Ʈ�κ��� Ÿ�ٱ����� ����
	const float kMinDistance = 0.1f;
	
	// Ÿ�ٿ� �������� �ʾ��� ��쿡�� �̵�
	if (toTarget.Length() > kMinDistance) {
		mEnemyMgr->mController->SetValue("Return", false);

		mObject->RotateTargetAxisY(target->GetPosition(), mEnemyMgr->mStat.RotationSpeed);
		mObject->Translate(mObject->GetLook(), mEnemyMgr->mStat.MoveSpeed * DeltaTime());
	}

	return BT::NodeState::Success;
}
