#pragma once

#pragma region Include
#include "Script_Weapon.h"
#pragma endregion


class Script_Weapon_Pistol : public Script_Weapon {
	COMPONENT(Script_Weapon_Pistol, Script_Weapon)

public:
	virtual void Start() override;

protected:
	virtual void CreateBulletPool() override;
	virtual void FireBullet() override;
};