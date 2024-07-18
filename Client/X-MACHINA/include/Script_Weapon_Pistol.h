#pragma once

#pragma region Include
#include "Script_Weapon.h"
#pragma endregion

// H-Lock pistol
class Script_Weapon_Pistol : public Script_BulletWeapon {
	COMPONENT(Script_Weapon_Pistol, Script_BulletWeapon)

private:
	static constexpr float mkBulletDamage = 8.f;
	static constexpr float mkBulletSpeed  = 40.f;
	static constexpr float mkRPM          = 350.f;

public:
	virtual WeaponType GetWeaponType() const override { return WeaponType::HandedGun; }
	virtual WeaponName GetWeaponName() const { return WeaponName::H_Lock; }

public:
	virtual void Awake() override;

protected:
	virtual float GetBulletSpeed() override { return mkBulletSpeed; }

private:
	virtual void InitValues() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const override;
	virtual void SetParticleSystemNames() override;
};