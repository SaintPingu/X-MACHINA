#pragma once

class UI;
class Script_Weapon;

class WeaponUI {
private:
	wptr<Script_Weapon>  mWeapon{};
	Vec2 mPos{};
	UI*	 mBackgroundUI{};
	UI*	 mBackgroundDecoUI{};
	UI*	 mWeaponUI{};
	UI*	 mWeaponMagUI{};
	UI*	 mWeaponMagOutlineUI{};

	std::string mPlayerName{};

public:
	WeaponUI(const Vec2& position, const Vec3& color, const std::string& playerName);
	
public:
	void SetWeapon(rsptr<Script_Weapon> weapon);
	void Update();

private:
	void Reset();
};