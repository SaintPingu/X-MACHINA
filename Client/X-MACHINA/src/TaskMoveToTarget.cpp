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
	Vec3 toTarget = mObject->GetPosition() - target->GetPosition();

	// ������Ʈ�� ���� �̵� ��ġ Ÿ���� �����ִ� ��� AStar�� ������Ѿ� �Ѵ�.
	Vec3 nextPos = mObject->GetLook() * mEnemyMgr->mMoveSpeed * DeltaTime() + mObject->GetPosition();

	// ���� ��ġ�� Ÿ���� ���� �ְų� ���� ��ã�� �������̶�� �������� �ʴ´�.
	if (scene->GetTileFromPos(nextPos) == Tile::Static || mEnemyMgr->mIsMoveToPath) {
		return BT::NodeState::Failure;
	}

	// ������Ʈ�κ��� Ÿ�ٱ����� ����
	const float kMinDistance = 0.1f;

	// Ÿ�ٿ� �������� �ʾ��� ��쿡�� �̵�
	if (toTarget.Length() > kMinDistance) {
		mObject->RotateTargetAxisY(target->GetPosition(), mEnemyMgr->mRotationSpeed * DeltaTime());
		mObject->Translate(mObject->GetLook(), mEnemyMgr->mMoveSpeed * DeltaTime());
	}

	return BT::NodeState::Success;
}
