#include "stdafx.h"
#include "Script_Enemy.h"

#include "Script_EnemyManager.h"
#include "Script_GroundObject.h"
#include "Script_PheroObject.h"
#include "Script_DefaultEnemyBT.h"

#include "Timer.h"
#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"

#include "XLManager.h"


void Script_Enemy::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_GroundObject>();
	mObject->AddComponent<Script_PheroObject>();
	mObject->AddComponent<Script_DefaultEnemyBT>();

	mEnemyMgr = mObject->AddComponent<Script_EnemyManager>();
	mEnemyMgr->mController = mObject->GetObj<GameObject>()->GetAnimator()->GetController();

	SetEnemyStat(mObject->GetName());
	SetMaxHP(mEnemyMgr->mStat.MaxHp);
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

void Script_Enemy::SetEnemyStat(const std::string& modelName)
{
	XLManager::I->Set(modelName, mEnemyMgr->mStat);
}
