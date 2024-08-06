#pragma once

#pragma region Include
#include "Script_Weapon.h"
#pragma endregion

// H-Lock pistol
class Script_Weapon_MineLauncher : public Script_BulletWeapon {
	COMPONENT(Script_Weapon_MineLauncher, Script_BulletWeapon)

private:
	static constexpr float mkMineSpeed       = 7.5f;
	static constexpr float mkExplosionDamage = 15.0f;
	static constexpr float mkRPM             = 0.1f;
	static constexpr int mkBulletCntPerMag   = 8;

public:
	virtual WeaponType GetWeaponType() const override { return WeaponType::MineLauncher; }
	virtual WeaponName GetWeaponName() const { return WeaponName::MineLauncher; }

public:
	virtual void Awake() override;

protected:
	virtual float GetBulletSpeed() override { return mkMineSpeed; }

private:
	virtual void InitValues() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const override;
	virtual void SetParticleSystemNames() override;

	virtual void CreateBulletPool() override;
};