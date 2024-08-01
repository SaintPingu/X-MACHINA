#pragma once

#pragma region Include
#include "Script_Weapon.h"
#pragma endregion

// base rifle class
class Script_Weapon_Shotgun abstract : public Script_BulletWeapon {
	COMPONENT_ABSTRACT(Script_Weapon_Shotgun, Script_BulletWeapon)

protected:
	static constexpr float mkBulletSpeed = 100.f;

public:
	virtual WeaponType GetWeaponType() const override { return WeaponType::ShotGun; }
	virtual WeaponName GetWeaponName() const abstract;

public:
	virtual void Awake() override;

protected:
	virtual void FireBullet() override;
};




class Script_Weapon_DBMS : public Script_Weapon_Shotgun {
	COMPONENT(Script_Weapon_DBMS, Script_Weapon_Shotgun)

private:
	static constexpr int mkBulletCntPerShot  = 15;
	static constexpr float mkBulletDamage    = 62.6f / mkBulletCntPerShot;
	static constexpr float mkRPM             = 200.f;

public:
	virtual WeaponName GetWeaponName() const { return WeaponName::DBMS; }

protected:
	virtual float GetBulletSpeed() override { return mkBulletSpeed; }

private:
	virtual void InitValues() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const override;
	virtual void SetParticleSystemNames() override;
};