#include "stdafx.h"
#include "CheckAttackRange.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Object.h"
#include "AnimatorController.h"


CheckAttackRange::CheckAttackRange(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}


BT::NodeState CheckAttackRange::Evaluate()
{
	sptr<Object> target = GetData("target");

	if (!target) {
		return BT::NodeState::Failure;
	}

	// TODO : ���� ������ ���� ���� ���� �ִϸ��̼� ��� ����(���� �ִϸ��̼��� �ӵ��� ��Ÿ���� ���� ��)
	if ((mObject->GetPosition() - target->GetPosition()).Length() < mEnemyMgr->mAttackRange) {

		mEnemyMgr->mController->SetValue("Walk", false);
		mEnemyMgr->mController->SetValue("Jump", true); // TODO : Attack���� �����ؾ� ��

		return BT::NodeState::Success;
	}


	return BT::NodeState::Failure;
}
