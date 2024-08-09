#include "stdafx.h"
#include "Script_BattleUI.h"

#include "Script_Weapon.h"

#include "WeaponUI.h"
#include "InputMgr.h"

void Script_BattleUI::Awake()
{
	base::Awake();

	// weapon UI
	{
		static constexpr Vec2 kStartOffset = Vec2(-450, -350);
		static constexpr Vec2 kGap = Vec2(300, 0);

		mWeaponUIs.emplace_back(std::make_shared<WeaponUI>(kStartOffset + (kGap * 0), Vec3(0.994160354f, 0.00894771889f, 0.0755915046f), L"πŒΩ¬¿Á", 25));
		mWeaponUIs.emplace_back(std::make_shared<WeaponUI>(kStartOffset + (kGap * 1), Vec3(0.579661787f, 0.785649717f, 0.320677072f), L"πÈΩ”ªÕ", 13));
		mWeaponUIs.emplace_back(std::make_shared<WeaponUI>(kStartOffset + (kGap * 2), Vec3(0.0594013520f, 0.659722447f, 0.532157719f), L"¿Â∞Ë≈¡", 1));
		mWeaponUIs.emplace_back(std::make_shared<WeaponUI>(kStartOffset + (kGap * 3), Vec3(0.0305042304f, 0.640245616f, 0.933808744f), L"Dragon Lee", 99));
	}
}

void Script_BattleUI::UpdateWeapon(int idx) const
{
	if (mWeaponUIs.size() <= idx) {
		return;
	}

	mWeaponUIs.at(idx)->Update();
}

void Script_BattleUI::SetWeapon(int idx, rsptr<Script_Weapon> weapon) const
{
	if (mWeaponUIs.size() <= idx) {
		return;
	}

	mWeaponUIs.at(idx)->SetWeapon(weapon);
}

int Script_BattleUI::CreateWeaponUI()
{
	return 0;
}
