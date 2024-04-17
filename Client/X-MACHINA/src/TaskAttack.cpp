#include "stdafx.h"
#include "TaskAttack.h"

#include "Script_EnemyManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "AnimatorController.h"


TaskAttack::TaskAttack(Object* object)
{
	mObject = object;
	mEnemyMgr = object->GetComponent<Script_EnemyManager>();
}


BT::NodeState TaskAttack::Evaluate()
{
	sptr<Object> target = GetData("target");

	mAttackAccTime += DeltaTime();
	if (mAttackAccTime >= mEnemyMgr->mAttackCoolTime) {

		mAttackAccTime = 0.f;
	}

	return BT::NodeState::Running;
}
