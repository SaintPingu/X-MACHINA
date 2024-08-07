#include "stdafx.h"
#include "WeaponUI.h"

#include "Script_Player.h"
#include "Component/UI.h"

WeaponUI::WeaponUI(const Vec2& position)
{
	mPos = position;
	mBackgroundUI = Canvas::I->CreateUI<UI>(0, "WeaponUI_Background", mPos);
	mBackgroundUI->SetOpacity(0.8f);

	mWeaponUI = Canvas::I->CreateUI<UI>(1, "WeaponUI_Skyline", mPos);
}

void WeaponUI::SetWeapon(Script_Weapon* weapon)
{
	mWeapon = weapon;
}

void WeaponUI::Update()
{
}
