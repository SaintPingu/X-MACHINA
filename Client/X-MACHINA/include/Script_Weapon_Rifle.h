#pragma once

#pragma region Include
#include "Script_Weapon.h"
#pragma endregion

// base rifle class
class Script_Weapon_Rifle abstract : public Script_BulletWeapon {
	COMPONENT_ABSTRACT(Script_Weapon_Rifle, Script_BulletWeapon)

protected:
	static constexpr float mkBulletSpeed = 40.f;

public:
	virtual void Awake() override;
};




class Script_Weapon_Skyline : public Script_Weapon_Rifle {
	COMPONENT(Script_Weapon_Skyline, Script_Weapon_Rifle)

private:
	static constexpr float mkBulletDamage = 5.4f;
	static constexpr float mkRPM          = 850.f;

private:
	virtual void InitValues() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const override;
};