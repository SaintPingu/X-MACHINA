#include "stdafx.h"
#include "WeaponUI.h"

#include "Script_Player.h"
#include "Script_Weapon.h"

#include "GameFramework.h"
#include "TextMgr.h"

#include "Component/UI.h"

WeaponUI::WeaponUI(const Vec2& position, const Vec3& color, const std::string& playerName)
{
	mPos = position;
	mBackgroundUI = Canvas::I->CreateUI<UI>(0, "WeaponUI_Background", mPos);

	mBackgroundDecoUI = Canvas::I->CreateUI<UI>(0, "WeaponUI_Background_deco", mPos);
	mBackgroundDecoUI->SetColor(color);

	mPlayerName = playerName;
	{
		static constexpr Vec2 kNameUIOffset{ -160, 45 };
		TextOption textOption{};
		//textOption.Font = "";
		textOption.FontSize = 25.f;
		textOption.FontStretch = TextFontStretch::EXTRA_EXPANDED;
		textOption.FontColor = TextFontColor::Type::White;
		textOption.FontWeight = TextFontWeight::HEAVY;
		textOption.VAlignment = TextParagraphAlign::Center;
		textOption.HAlignment = TextAlignType::Leading;

		TextMgr::I->CreateText(mPlayerName, mPos + kNameUIOffset + Vec2(GameFramework::I->GetWindowSize().x / 2.f, 0), textOption);
	}
}

void WeaponUI::SetWeapon(rsptr<Script_Weapon> weapon)
{
	static const std::unordered_map<WeaponName, std::string> kWeaponUIMap{
		{ WeaponName::H_Lock, "WeaponUI_H_Lock"},
		{ WeaponName::SkyLine, "WeaponUI_Skyline"},
		{ WeaponName::DBMS, "WeaponUI_DBMS"},
		{ WeaponName::Burnout, "WeaponUI_Burnout"},
		{ WeaponName::PipeLine, "WeaponUI_PipeLine"},
		{ WeaponName::MineLauncher, "WeaponUI_MineLauncher"},
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
	mWeaponUI = Canvas::I->CreateUI<UI>(1, weaponUIName, weaponUIPos, Vec2(190, 90) * 0.9f);
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
