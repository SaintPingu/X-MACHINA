#pragma once

#pragma region Include
#include "Script_Weapon.h"
#pragma endregion


class Script_Weapon_Pistol : public Script_Weapon {
	COMPONENT(Script_Weapon_Pistol, Script_Weapon)

private:
	static constexpr float kBulletDamage = 8.f;
	static constexpr float kBulletSpeed  = 30.f;
	static constexpr float kRPM          = 350.f;

public:
	virtual void Awake() override;

protected:
	virtual void CreateBulletPool() override;
	virtual void FireBullet() override;

private:
	virtual void InitValues() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const override;
};