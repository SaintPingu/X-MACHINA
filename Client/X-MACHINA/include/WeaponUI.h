#pragma once

class UI;
class Script_Weapon;

class WeaponUI {
private:
	Vec2			mPos{};
	Script_Weapon*  mWeapon{};
	UI*				mBackgroundUI{};
	UI*				mWeaponUI{};

public:
	WeaponUI(const Vec2& position);
	
public:
	void SetWeapon(Script_Weapon* weapon);
	void Update();
};

