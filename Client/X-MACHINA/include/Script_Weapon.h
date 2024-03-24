#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Transform;
class GridObject;
#pragma endregion


enum class WeaponType {
	HandedGun = 0,
	AssaultRifle,
	LightingGun,
	GatlinGun,
	ShotGun,
	MissileLauncher,
	_count
};
constexpr size_t gkWeaponTypeCnt = static_cast<size_t>(WeaponType::_count);

class Script_Weapon : public Component {
	COMPONENT(Script_Weapon, Component)

};