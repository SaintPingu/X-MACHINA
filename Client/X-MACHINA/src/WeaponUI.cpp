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
		static constexpr Vec2 kNameUIOffset{ -150, 40 };
		TextOption textOption{};
		//textOption.Font = "";
		textOption.FontSize = 18.f;
		textOption.FontStretch = TextFontStretch::EXTRA_EXPANDED;
		textOption.FontColor = TextFontColor::Type::Gray;
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

	static const std::unordered_map<WeaponName, std::string> kWeaponMagUIMap{
		{ WeaponName::H_Lock, "WeaponMagUI_H_Lock"},	
		{ WeaponName::SkyLine, "WeaponMagUI_Skyline"},
		{ WeaponName::DBMS, "WeaponMagUI_DBMS"},
		{ WeaponName::Burnout, "WeaponMagUI_Burnout"},
		{ WeaponName::PipeLine, "WeaponMagUI_PipeLine"},
		{ WeaponName::MineLauncher, "WeaponMagUI_MineLauncher"},
	};

	static constexpr Vec2 kWeaponUIPosOffset{ -30, -20 };
	static constexpr Vec2 kWeaponMagUIPosOffset{ 100, -20 };

	Reset();

	WeaponName weaponName = weapon->GetWeaponName();
	// weapon //
	if(!kWeaponUIMap.count(weaponName)) {
		return;
	}

	mWeapon = weapon;

	const std::string& weaponUIName = kWeaponUIMap.at(weaponName);
	mWeaponUI = Canvas::I->CreateUI<UI>(1, weaponUIName, mPos + kWeaponUIPosOffset, Vec2(190, 90) * 0.9f);

	// mag //
	if (!kWeaponMagUIMap.count(weaponName)) {
		return;
	}

	const std::string& weaponMagUIName = kWeaponMagUIMap.at(weaponName);
	mWeaponMagUI = Canvas::I->CreateUI<UI>(1, weaponMagUIName, mPos + kWeaponMagUIPosOffset);
	const std::string outlineName = weaponMagUIName + "_outline";
	mWeaponMagOutlineUI = Canvas::I->CreateUI<UI>(2, outlineName, mPos + kWeaponMagUIPosOffset);
	
	Update();
}

void WeaponUI::Update()
{
	if (mWeaponMagUI) {
		const auto& weapon = mWeapon.lock();
		mWeaponMagUI->mObjectCB.SliderValueY = (float)weapon->GetCurBulletCnt() / weapon->GetMaxBulletCnt();
		if (mWeaponMagUI->mObjectCB.SliderValueY < 0.2f) {
			mWeaponMagOutlineUI->SetColor(Vec3(1.0f, 0, 0));
		}
		else {
			mWeaponMagOutlineUI->RemoveColor();
		}
	}
}

void WeaponUI::Reset()
{
	mWeapon.reset();
	if (mWeaponUI) {
		mWeaponUI->Remove();
		mWeaponUI = nullptr;
	}

	if (mWeaponMagUI) {
		mWeaponMagUI->Remove();
		mWeaponMagUI = nullptr;
	}
	if (mWeaponMagOutlineUI) {
		mWeaponMagOutlineUI->Remove();
		mWeaponMagOutlineUI = nullptr;
	}
}
