#include "stdafx.h"
#include "WeaponUI.h"

#include "Script_Player.h"
#include "Script_Weapon.h"

#include "Component/UI.h"

WeaponUI::WeaponUI(const Vec2& position)
{
	mPos = position;
	mBackgroundUI = Canvas::I->CreateUI<UI>(0, "WeaponUI_Background", mPos);
	mBackgroundUI->SetOpacity(0.6f);
}

void WeaponUI::SetWeapon(rsptr<Script_Weapon> weapon)
{
	static const std::unordered_map<WeaponName, std::string> kWeaponUIMap{
		{ WeaponName::SkyLine, "WeaponUI_Skyline"},
	};

	static constexpr Vec2 kWeaponUIPosOffset{ -10, -20 };

	Reset();

	WeaponName weaponName = weapon->GetWeaponName();

	if(!kWeaponUIMap.count(weaponName)) {
		return;
	}

	mWeapon = weapon;

	const std::string& weaponUIName = kWeaponUIMap.at(weaponName);
	const Vec2 weaponUIPos = mPos + kWeaponUIPosOffset;
	mWeaponUI = Canvas::I->CreateUI<UI>(1, weaponUIName, weaponUIPos);
}

void WeaponUI::Update()
{
}

void WeaponUI::Reset()
{
	mWeapon.reset();
	if (mWeaponUI) {
		mWeaponUI->Remove();
		mWeaponUI = nullptr;
	}
}
