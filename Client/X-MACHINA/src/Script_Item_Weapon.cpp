#include "stdafx.h"
#include "Script_Item.h"

#include "Script_Player.h"
#include "Script_Weapon.h"
#include "Script_Weapon_Pistol.h"
#include "Script_Weapon_Rifle.h"
#include "Script_Weapon_Shotgun.h"
#include "Script_Weapon_Sniper.h"
#include "Script_Weapon_MissileLauncher.h"
#include "Script_Weapon_MineLauncher.h"

#include "Component/Rigidbody.h"

#include "Timer.h"
#include "Object.h"

#include "ClientNetwork/Contents/Script_NetworkRemotePlayer.h"


void Script_Item_Weapon::Awake()
{
	base::Awake();

	mMaxFloatingSpeed = 0.25f;
}

void Script_Item_Weapon::Animate()
{
	if (!mDroped) {
		return;
	}

	base::Animate();

	float floatingSpeed = mMaxFloatingSpeed * sin(mDeltaTime * XM_PI);

	mObject->SetPositionY(mObject->GetPosition().y + (floatingSpeed * mSign * DeltaTime()));

	mDeltaTime += DeltaTime();
	if (mDeltaTime >= 1.f) {
		mDeltaTime = 0.f;
		mSign *= -1;
	}
}


void Script_Item_Weapon::StartOpen()
{
}

void Script_Item_Weapon::StartDrop()
{
	mDroped = true;

	mObject->SetPositionY(0.5f);
	mObject->mObjectCB.UseOutline = true;
}

void Script_Item_Weapon::SetWeapon(WeaponName weaponName)
{
	if (mObject->GetComponent<Script_Weapon>()) {
		return;
	}

	switch (weaponName) {
	case WeaponName::H_Lock:
		mObject->AddComponent<Script_Weapon_Pistol>();
		break;
	case WeaponName::SkyLine:
		mObject->AddComponent<Script_Weapon_Skyline>();
		break;
	case WeaponName::DBMS:
		mObject->AddComponent<Script_Weapon_DBMS>();
		break;
	case WeaponName::Burnout:
		mObject->AddComponent<Script_Weapon_Burnout>();
		break;
	case WeaponName::PipeLine:
		mObject->AddComponent<Script_Weapon_PipeLine>();
		break;
	case WeaponName::MineLauncher:
		mObject->AddComponent<Script_Weapon_MineLauncher>();
		break;
	default:
		assert(0);
		break;
	}
}

bool Script_Item_Weapon::InteractOK(Object* user)
{
	const auto& weapon = mObject->GetComponent<Script_Weapon>();

	if (weapon) {
		mObject->SetTag(ObjectTag::Untagged);
		mObject->SetActive(false);
		mObject->mObjectCB.UseOutline = false;
	}
	else {
		std::cout << "[WARNING] (weapon item or player) has no script\n";
		mObject->Destroy();
		return true;
	}


	const auto& player = user->GetComponent<Script_GroundPlayer>();
	if (player) {
		player->TakeWeapon(weapon);
	}
	else {
		const auto& removeplayer = user->GetComponent<Script_NetworkRemotePlayer>();
		if (removeplayer) {
			removeplayer->TakeWeapon(weapon);
		}
	}

	mDroped = false;

	return true;
}

void Script_Item_Weapon::Throw(const Vec3& pos)
{
	mObject->DetachParent(false);
	mObject->SetPosition(pos);
	mObject->SetLocalRotation(Vec3(0, 90, 0));
	mObject->SetTag(ObjectTag::Item);

	const auto& weapon = mObject->GetComponent<Script_Weapon>();
	if (weapon) {
		weapon->StopFire();
		weapon->SetOwner(nullptr);
	}

	StartDrop();
	mObject->SetActive(true);
}
