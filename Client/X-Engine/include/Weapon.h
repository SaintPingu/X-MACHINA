#pragma once

class Transform;
class GridObject;

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

class Weapon {
private:
	const sptr<GridObject> mObject;

public:
	Weapon(rsptr<GridObject> object, Transform* parent);
	virtual ~Weapon() = default;

public:
	void Enable();
	void Disable();
};