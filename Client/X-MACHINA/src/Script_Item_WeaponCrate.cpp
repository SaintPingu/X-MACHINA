#include "stdafx.h"
#include "Script_Item.h"

#include "Script_Weapon.h"
#include "Script_Weapon_Pistol.h"
#include "Script_Weapon_Rifle.h"
#include "Script_Weapon_Shotgun.h"
#include "Script_Weapon_Sniper.h"
#include "Script_Weapon_MissileLauncher.h"

#include "ScriptExporter.h"
#include "BattleScene.h"
#include "Timer.h"
#include "Object.h"

#include "ClientNetwork/Contents/ClientNetworkManager.h"


void Script_Item_WeaponCrate::Awake()
{
	base::Awake();

	mObject->RemoveComponent<ScriptExporter>();
	mCap = mObject->FindFrame("box_cap");
}

void Script_Item_WeaponCrate::Animate()
{
	base::Animate();

	if (!mIsOpend) {
		return;
	}

	constexpr float openSpeed = 250.f;
	constexpr float maxPitch = 190.f;

	if (mCapPitch > maxPitch) {
		return;
	}

	const float openAmount = DeltaTime() * openSpeed;

	mCapPitch += openAmount;
	mCap->Rotate(Vector3::Down, openAmount);
}

void Script_Item_WeaponCrate::OnCollisionEnter(Object& other)
{
	if (mIsOpend) {
		return;
	}

	base::OnCollisionEnter(other);
}

void Script_Item_WeaponCrate::LoadData(rsptr<ScriptExporter> exporter)
{
	std::string weaponName;
	int id{};
	exporter->GetData("Name", weaponName);
	exporter->GetData("ID", id);
	mObject->SetID(id);
	CLIENT_NETWORK->AddItem(id, this);
}

void Script_Item_WeaponCrate::DisableInteract()
{
	base::DisableInteract();
}

bool Script_Item_WeaponCrate::Interact()
{
	if (mIsOpend) {
		return false;
	}
	DisableInteract();

	return true;
}