#include "stdafx.h"
#include "Script_Item.h"

#include "Script_Weapon.h"

#include "ScriptExporter.h"
#include "Scene.h"
#include "Timer.h"
#include "Object.h"

void Script_Item_WeaponCrate::Awake()
{
	base::Awake();

	mObject->RemoveComponent<ScriptExporter>();
	mCap = mObject->FindFrame("box_cap");
}

void Script_Item_WeaponCrate::Animate()
{
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

void Script_Item_WeaponCrate::LoadData(rsptr<ScriptExporter> exporter)
{
	std::string weaponName;
	exporter->GetData("WeaponName", weaponName);
	mWeaponName = gkWeaponNameMap.at(Hash(weaponName));
}

WeaponName Script_Item_WeaponCrate::Interact()
{
	if (mIsOpend) {
		return WeaponName::None;
	}

	mIsOpend = true;
	return mWeaponName;
}