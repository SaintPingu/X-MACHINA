#include "stdafx.h"
#include "Script_Deus_Phase_1.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

#include "Object.h"
#include "BattleScene.h"

void Script_Deus_Phase_1::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Deus_Phase_1::MeleeAttackCallback, this), 16);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack2AnimName)->AddStartCallback(std::bind(&Script_Deus_Phase_1::RangeAttackCallback, this));
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddStartCallback(std::bind(&Script_Deus_Phase_1::ExplodeAttackCallback, this));

	auto weapon = BattleScene::I->Instantiate("Deus_Phase_1_Rifle_Rust");
	weapon->SetLocalTransform(Matrix::Identity);
	mObject->FindFrame("RefPosGun")->SetChild(weapon->GetObj<Transform>()->GetShared());
}

void Script_Deus_Phase_1::MeleeAttackCallback()
{
}

void Script_Deus_Phase_1::RangeAttackCallback()
{
}

void Script_Deus_Phase_1::ExplodeAttackCallback()
{
}
