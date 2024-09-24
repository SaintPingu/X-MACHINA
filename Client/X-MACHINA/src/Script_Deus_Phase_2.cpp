#include "stdafx.h"
#include "Script_Deus_Phase_2.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

#include "Object.h"
#include "BattleScene.h"

void Script_Deus_Phase_2::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Deus_Phase_2::AttackCallback, this), 26);
}

void Script_Deus_Phase_2::Start()
{
	mLeftWeapon = BattleScene::I->Instantiate("Deus_Phase_2_Sword");
	mLeftWeapon->SetLocalTransform(Matrix::Identity);
	mObject->FindFrame("RefPosSword_L")->SetChild(mLeftWeapon->GetObj<Transform>()->GetShared());

	mRightWeapon = BattleScene::I->Instantiate("Deus_Phase_2_Sword_Rust");
	mRightWeapon->SetLocalTransform(Matrix::Identity);
	mObject->FindFrame("RefPosSword_R")->SetChild(mRightWeapon->GetObj<Transform>()->GetShared());
}

void Script_Deus_Phase_2::OnDestroy()
{
	base::OnDestroy();

	mLeftWeapon->Destroy();
	mRightWeapon->Destroy();
}

