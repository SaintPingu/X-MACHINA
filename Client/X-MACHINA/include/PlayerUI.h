#pragma once

class UI;
class Script_Weapon;
class TextBox;

class PlayerUI {
private:
	wptr<Script_Weapon>  mWeapon{};
	Vec2 mPos{};
	UI*	 mBackgroundUI{};
	UI*	 mBackgroundDecoUI{};
	UI*	 mWeaponUI{};
	UI*	 mWeaponMagUI{};
	UI*	 mWeaponMagOutlineUI{};

	TextBox* mNameText{};
	TextBox* mLevelText{};

public:
	PlayerUI(const Vec2& position, const Vec3& color, const std::wstring& playerName, int playerLevel);
	
public:
	void SetWeapon(rsptr<Script_Weapon> weapon);
	void SetPosition(const Vec2& position);
	void SetColor(const Vec3& color);

	void Update();

private:
	void Reset();
};