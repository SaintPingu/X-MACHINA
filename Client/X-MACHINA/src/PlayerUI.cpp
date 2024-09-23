#include "stdafx.h"
#include "PlayerUI.h"

#include "Script_Player.h"
#include "Script_Weapon.h"

#include "GameFramework.h"
#include "TextMgr.h"
#include "InputMgr.h"
#include "Timer.h"

#include "Component/UI.h"
#include "Component/Camera.h"

PlayerUI::PlayerUI(const Vec2& position, const Vec3& color, const Object* player, const std::wstring& playerName, int playerLevel)
	: mPlayer(player)
{
	mBackgroundUI = Canvas::I->CreateUI<UI>(0, "WeaponUI_Background");


	mBackgroundDecoUI = Canvas::I->CreateUI<UI>(1, "WeaponUI_Background_deco");
	mBackgroundDecoUI->SetColor(color);

	{
		TextOption textOption{};
		textOption.Font = "Malgun Gothic";
		textOption.FontSize = 18.f;
		textOption.FontColor = TextFontColor::Type::Gray;
		textOption.FontWeight = TextFontWeight::DEMI_BOLD;
		textOption.HAlignment = TextAlignType::Leading;

		mNameText = TextMgr::I->CreateText(WstringToString(playerName), Vec2::Zero, textOption);
	}

	{
		TextOption textOption{};
		textOption.Font = "Malgun Gothic";
		textOption.FontSize = 25.f;
		textOption.FontColor = TextFontColor::Type::DarkGoldenrod;
		textOption.FontWeight = TextFontWeight::EXTRA_BOLD;
		textOption.HAlignment = TextAlignType::Center;

		const std::string levelText = std::to_string(playerLevel);
		mLevelText = TextMgr::I->CreateText(levelText, Vec2::Zero, textOption);
	}

	{
		TextOption textOption{};
		textOption.Font = "Malgun Gothic";
		textOption.FontSize = 16.f;
		textOption.FontColor = TextFontColor::Type::GhostWhite;
		textOption.FontWeight = TextFontWeight::DEMI_BOLD;

		mNameUI = TextMgr::I->CreateText(WstringToString(playerName), Vec2::Zero, textOption);
	}

	SetPosition(position);
}

PlayerUI::~PlayerUI()
{
	Reset();

	if (mNameUI) {
		TextMgr::I->RemoveTextBox(mNameUI);
		mNameUI = nullptr;
	}

	mBackgroundDecoUI->Remove();
	mBackgroundUI->Remove();
	TextMgr::I->RemoveTextBox(mNameText);
	TextMgr::I->RemoveTextBox(mLevelText);
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
	mWeaponUI = Canvas::I->CreateUI<UI>(1, weaponUIName);

	// mag //
	if (!kWeaponMagUIMap.count(weaponName)) {
		return;
	}

	const std::string& weaponMagUIName = kWeaponMagUIMap.at(weaponName);
	mWeaponMagUI = Canvas::I->CreateUI<UI>(1, weaponMagUIName);
	const std::string outlineName = weaponMagUIName + "_outline";
	mWeaponMagOutlineUI = Canvas::I->CreateUI<UI>(2, outlineName);

	UpdateWeaponUIPos();
	Update();
}

void PlayerUI::SetPosition(const Vec2& position)
{
	mPos = position;
	mBackgroundUI->SetPosition(mPos);

	static constexpr Vec2 kDecoUIOffset{ -127, -0.5f };
	mBackgroundDecoUI->SetPosition(mPos + kDecoUIOffset);

	static constexpr Vec2 kNameUIOffset{ -80, 30 };
	mNameText->SetPosition(mPos + kNameUIOffset + Vec2(GameFramework::I->GetWindowSize().x / 2.f, 0));

	static constexpr Vec2 kLevelUIOffset{ -100, 33 };
	mLevelText->SetPosition(mPos + kLevelUIOffset);

	UpdateWeaponUIPos();
}

void PlayerUI::SetColor(const Vec3& color)
{
	mBackgroundDecoUI->SetColor(color);
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

void PlayerUI::UpdateSimple()
{
	if (mNameUI) {
		Vec2 pos = MAIN_CAMERA->WorldToScreenPoint(mPlayer->GetPosition());
		static constexpr int yOffset = 87;
		pos.y += yOffset;

		mNameUI->SetPosition(pos);
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

void PlayerUI::UpdateWeaponUIPos()
{
	static constexpr Vec2 kWeaponUIPosOffset{ -15, -20 };
	static constexpr Vec2 kWeaponMagUIPosOffset{ 90, -20 };

	if (mWeaponUI) {
		mWeaponUI->SetPosition(mPos + kWeaponUIPosOffset);
	}

	if (mWeaponMagUI) {
		mWeaponMagUI->SetPosition(mPos + kWeaponMagUIPosOffset);
	}
	if (mWeaponMagOutlineUI) {
		mWeaponMagOutlineUI->SetPosition(mPos + kWeaponMagUIPosOffset);
	}
}
