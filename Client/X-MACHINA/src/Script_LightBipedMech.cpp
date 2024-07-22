#include "stdafx.h"
#include "Script_LightBipedMech.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"
#include "BattleScene.h"
#include "Object.h"


void Script_LightBipedMech::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.AttackAnimName)->AddCallback(std::bind(&Script_LightBipedMech::AttackCallback, this), 20);

	mWeapon = BattleScene::I->Instantiate("SM_Phaser");
	Transform* weaponPos = mObject->FindFrame("RefPos_Weapon");
	weaponPos->SetChild(mWeapon->GetShared());
	mWeapon->Rotate(Vector3::Right, 90);
}

void Script_LightBipedMech::OnDestroy()
{
	base::OnDestroy();

	mWeapon->OnDestroy();
}