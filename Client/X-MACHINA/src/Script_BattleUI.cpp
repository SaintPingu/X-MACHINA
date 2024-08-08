#include "stdafx.h"
#include "Script_BattleUI.h"

#include "Script_Weapon.h"

#include "WeaponUI.h"

void Script_BattleUI::Awake()
{
	base::Awake();

	mWeaponUIs.emplace_back(std::make_shared<WeaponUI>(Vec2(-700, -350), Vec3(0.8f, 0.8f, 0.2f), "TestPlayer"));
}

void Script_BattleUI::Update()
{
	base::Update();

	for (const auto& weaponUI : mWeaponUIs) {
		weaponUI->Update();
	}
}

void Script_BattleUI::SetWeapon(int idx, rsptr<Script_Weapon> weapon) const
{
	if (mWeaponUIs.size() <= idx) {
		return;
	}

	mWeaponUIs.at(idx)->SetWeapon(weapon);
}