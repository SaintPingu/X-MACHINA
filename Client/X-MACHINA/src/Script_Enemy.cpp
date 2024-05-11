#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_EnemyManager.h"
#include "Script_GroundObject.h"

#include "Timer.h"
#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"


void Script_Enemy::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_GroundObject>();

	mEnemyMgr = mObject->AddComponent<Script_EnemyManager>();
	mEnemyMgr->mController = mObject->GetObj<GameObject>()->GetAnimator()->GetController();
}

void Script_Enemy::Update()
{
	base::Update();

	mObject->mObjectCB.RimFactor = max(mObject->mObjectCB.RimFactor - DeltaTime(), 0.f);
}

void Script_Enemy::Attack()
{
}

void Script_Enemy::Death()
{
}

bool Script_Enemy::Hit(float damage)
{
	bool res = base::Hit(damage);

	mObject->mObjectCB.RimFactor = 0.7f;

	return res;
}
