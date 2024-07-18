#pragma once

#pragma region Include
#include "Script_Weapon.h"
#pragma endregion

// base rifle class
class Script_Weapon_Sniper abstract : public Script_BulletWeapon {
	COMPONENT_ABSTRACT(Script_Weapon_Sniper, Script_BulletWeapon)

protected:
	static constexpr float mkBulletSpeed = 100.f;

public:
	virtual WeaponType GetWeaponType() const override { return WeaponType::Sniper; }
	virtual WeaponName GetWeaponName() const abstract;

public:
	virtual void Awake() override;
};




class Script_Weapon_PipeLine : public Script_Weapon_Sniper {
	COMPONENT(Script_Weapon_PipeLine, Script_Weapon_Sniper)

private:
	static constexpr float mkBulletDamage    = 78.3f;
	static constexpr float mkRPM             = 55.f;

public:
	virtual WeaponName GetWeaponName() const { return WeaponName::PipeLine; }

protected:
	virtual float GetBulletSpeed() override { return mkBulletSpeed; }

private:
	virtual void InitValues() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const override;
	virtual void SetParticleSystemNames() override;
};