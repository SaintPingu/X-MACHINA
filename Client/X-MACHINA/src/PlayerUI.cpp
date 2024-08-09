#include "stdafx.h"
#include "PlayerUI.h"

#include "Script_Player.h"
#include "Script_Weapon.h"

#include "GameFramework.h"
#include "TextMgr.h"
#include "InputMgr.h"
#include "Timer.h"

#include "Component/UI.h"

PlayerUI::PlayerUI(const Vec2& position, const Vec3& color, const std::wstring& playerName, int playerLevel)
{
	mPos = position;
	mBackgroundUI = Canvas::I->CreateUI<UI>(0, "WeaponUI_Background", mPos);


	static constexpr Vec2 kDecoUIOffset{ -127, -0.5f };
	mBackgroundDecoUI = Canvas::I->CreateUI<UI>(1, "WeaponUI_Background_deco", mPos + kDecoUIOffset);
	mBackgroundDecoUI->SetColor(color);

	{
		static constexpr Vec2 kNameUIOffset{ -80, 30 };
		TextOption textOption{};
		textOption.Font = "Malgun Gothic";
		textOption.FontSize = 18.f;
		textOption.FontColor = TextFontColor::Type::Gray;
		textOption.FontWeight = TextFontWeight::DEMI_BOLD;
		textOption.HAlignment = TextAlignType::Leading;

		TextMgr::I->CreateText(WstringToString(playerName), mPos + kNameUIOffset + Vec2(GameFramework::I->GetWindowSize().x / 2.f, 0), textOption);
	}

	{
		static constexpr Vec2 kLevelUIOffset{ -100, 33 };
		TextOption textOption{};
		textOption.Font = "Malgun Gothic";
		textOption.FontSize = 25.f;
		textOption.FontColor = TextFontColor::Type::DarkGoldenrod;
		textOption.FontWeight = TextFontWeight::EXTRA_BOLD;
		textOption.HAlignment = TextAlignType::Center;

		const std::string levelText = std::to_string(playerLevel);
		TextMgr::I->CreateText(levelText, mPos + kLevelUIOffset, textOption);
	}
}

void PlayerUI::SetWeapon(rsptr<Script_Weapon> weapon)
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

	static constexpr Vec2 kWeaponUIPosOffset{ -15, -20 };
	static constexpr Vec2 kWeaponMagUIPosOffset{ 90, -20 };

	Reset();

	if (!weapon) {
		return;
	}

	WeaponName weaponName = weapon->GetWeaponName();
	// weapon //
	if (!kWeaponUIMap.count(weaponName)) {
		return;
	}

	mWeapon = weapon;

	const std::string& weaponUIName = kWeaponUIMap.at(weaponName);
	mWeaponUI = Canvas::I->CreateUI<UI>(1, weaponUIName, mPos + kWeaponUIPosOffset);

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

void PlayerUI::Update()
{
	constexpr float kOutOfMag_t = 0.2f;
	constexpr float kMidMag_t = 0.5f;
	constexpr Vec3 kOutOfMagColor = Vec3(1.f, 0.f, 0.f);
	constexpr Vec3 kMidMagColor = Vec3(1.f, 0.5f, 0.2f);

	if (mWeaponMagUI) {
		const auto& weapon = mWeapon.lock();
		mWeaponMagUI->mObjectCB.SliderValueY = (float)weapon->GetCurBulletCnt() / weapon->GetMaxBulletCnt();
		if (mWeaponMagUI->mObjectCB.SliderValueY <= kOutOfMag_t) {
			mWeaponMagOutlineUI->SetColor(kOutOfMagColor);
		}
		else if (mWeaponMagUI->mObjectCB.SliderValueY <= kMidMag_t) {
			mWeaponMagOutlineUI->SetColor(kMidMagColor);
		}
		else {
			mWeaponMagOutlineUI->RemoveColor();
		}
	}
}

void PlayerUI::Reset()
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
