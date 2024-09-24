#include "stdafx.h"
#include "Script_LightBipedMech.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"
#include "BattleScene.h"
#include "Object.h"

#include "Script_Weapon_MissileLauncher.h"

void Script_LightBipedMech::Awake()
{
	base::Awake();
}

void Script_LightBipedMech::Start()
{
	mWeapon = BattleScene::I->Instantiate("SM_Phaser");
	mObject->FindFrame("RefPos_Weapon")->SetChild(mWeapon->GetObj<Transform>()->GetShared());
	mWeapon->Rotate(Vector3::Right, 90);

	mWeaponScript = mWeapon->GetObj<Object>()->AddComponent<Script_Weapon_LightBiped>();
}

void Script_LightBipedMech::StartAttack()
{
	mWeaponScript->FireBullet();
}

void Script_LightBipedMech::OnDestroy()
{
	base::OnDestroy();

	mWeapon->Destroy();
}