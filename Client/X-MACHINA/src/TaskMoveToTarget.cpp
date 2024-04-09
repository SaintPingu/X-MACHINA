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

	// ������Ʈ�κ��� Ÿ�ٱ����� ����
	Vec3 toTarget =  mObject->GetPosition() - target->GetPosition();

	// Ÿ�����κ��� ������Ʈ�� ������ ���.
	Ray r{ target->GetPosition(), XMVector3Normalize(toTarget)};

	// �ش� ������ ���� �ٸ� Static ������Ʈ�� �Ÿ��� Ÿ�ٱ����� �Ÿ����� ����� ��� ���� �����ִ� ����̴�.
	if (scene->CheckCollisionsRay(r) < toTarget.Length()) {
		return BT::NodeState::Failure;
	}

	// ������Ʈ�κ��� Ÿ�ٱ����� ����
	const float kMinDistance = 0.1f;
	
	// �� ���̶� �ش� ��尡 ����ƴٸ� ���� ��ã�� ��θ� ���� �ϱ� ���� �÷���
	mEnemyMgr->mIsMoveToPath = false;

	// Ÿ�ٿ� �������� �ʾ��� ��쿡�� �̵�
	if (toTarget.Length() > kMinDistance) {
		mObject->RotateTargetAxisY(target->GetPosition(), mEnemyMgr->mRotationSpeed * DeltaTime());
		mObject->Translate(mObject->GetLook(), mEnemyMgr->mMoveSpeed * DeltaTime());
	}

	return BT::NodeState::Success;
}
