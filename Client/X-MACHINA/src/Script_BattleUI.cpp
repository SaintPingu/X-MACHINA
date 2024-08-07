#include "stdafx.h"
#include "Script_BattleUI.h"

#include "WeaponUI.h"

void Script_BattleUI::Awake()
{
	base::Awake();

	mWeaponUIs.emplace_back(std::make_shared<WeaponUI>(Vec2(-700, -350)));
}

void Script_BattleUI::Update()
{
	base::Update();

	for (const auto& weaponUI : mWeaponUIs) {
		weaponUI->Update();
	}
}
