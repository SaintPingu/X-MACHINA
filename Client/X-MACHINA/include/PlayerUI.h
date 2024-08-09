#pragma once

class UI;
class Script_Weapon;

class PlayerUI {
private:
	wptr<Script_Weapon>  mWeapon{};
	Vec2 mPos{};
	UI*	 mBackgroundUI{};
	UI*	 mBackgroundDecoUI{};
	UI*	 mWeaponUI{};
	UI*	 mWeaponMagUI{};
	UI*	 mWeaponMagOutlineUI{};

public:
	PlayerUI(const Vec2& position, const Vec3& color, const std::wstring& playerName, int playerLevel);
	
public:
	void SetWeapon(rsptr<Script_Weapon> weapon);
	void Update();
private:
	void Reset();
};