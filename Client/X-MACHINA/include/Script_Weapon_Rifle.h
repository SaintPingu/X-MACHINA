#pragma once

#pragma region Include
#include "Script_Weapon.h"
#pragma endregion

// base rifle class
class Script_Weapon_Rifle abstract : public Script_BulletWeapon {
	COMPONENT_ABSTRACT(Script_Weapon_Rifle, Script_BulletWeapon)

protected:
	Vec2 mCurErr{};
	float mMaxErrX{};
	float mMaxErrY{};

	float mErrRecoverSpeed = 5.f;
	float mErrIncAmount    = 0.5f;

public:
	virtual WeaponType GetWeaponType() const override { return WeaponType::AssaultRifle; }
	virtual WeaponName GetWeaponName() const abstract;

public:
	virtual void Awake() override;
	virtual void Update() override;

protected:
	virtual void FireBullet() override;
};




class Script_Weapon_Skyline : public Script_Weapon_Rifle {
	COMPONENT(Script_Weapon_Skyline, Script_Weapon_Rifle)

private:
	static constexpr float mkBulletSpeed = 50.f;
	static constexpr float mkBulletDamage = 5.4f;
	static constexpr float mkRPM          = 850.f;

public:
	virtual WeaponName GetWeaponName() const { return WeaponName::SkyLine; }

protected:
	virtual float GetBulletSpeed() override { return mkBulletSpeed; }

private:
	virtual void InitValues() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const override;
	virtual void SetParticleSystemNames() override;
};