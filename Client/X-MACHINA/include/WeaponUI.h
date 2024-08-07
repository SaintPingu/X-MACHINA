#pragma once

class UI;
class Script_Weapon;

class WeaponUI {
private:
	wptr<Script_Weapon>  mWeapon{};
	Vec2 mPos{};
	UI*	 mBackgroundUI{};
	UI*	 mWeaponUI{};

public:
	WeaponUI(const Vec2& position);
	
public:
	void SetWeapon(rsptr<Script_Weapon> weapon);
	void Update();

private:
	void Reset();
};

