#include "stdafx.h"
#include "Script_Onyscidus.h"

#include "Script_EnemyManager.h"
#include "Script_PheroObject.h"

void Script_Onyscidus::Awake()
{
	base::Awake();

	SetMaxHP(200);

	mEnemyMgr->mRotationSpeed = 150.f;
	mEnemyMgr->mMoveSpeed = 2.5f;
	mEnemyMgr->mDetectionRange = 15.f;
	mEnemyMgr->mAttackRange = 4.f;
	mEnemyMgr->mGetHitName = "BlockIdle";
	mEnemyMgr->mAttackName = "2HitComboClawsAttack";

	mObject->GetComponent<Script_PheroObject>()->SetPheroObjectLevel(1);
}