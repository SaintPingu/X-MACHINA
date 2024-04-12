#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Transform;
class GridObject;
class ObjectPool;
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

class Script_Weapon abstract : public Component {
	COMPONENT_ABSTRACT(Script_Weapon, Component)

protected:
	Transform* mMuzzle{};		// 총구
	sptr<ObjectPool> mBulletPool{};

	float mBulletDamage{};		// 총알 대미지
	float mBulletSpeed{};		// 총알 속도

	float mMaxFireDelay{};		// 총알 발사 딜레이
	float mCurFireDelay{};		// 현재 발사 딜레이

	float mMaxReloadTime{};		// 장전 시간
	float mCurReloadTime{};		// 현재 장전 시간

	float mMaxDistance{};		// 최대 사거리

	int mMaxMag{};				// 최대 장탄 수
	int mCurMag{};				// 현재 장탄 수

	bool  mIsShooting{};		// 발사 버튼을 누르고 있는가?

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	Transform* GetMuzzle() const { return mMuzzle; }

public:
	void StartFire() { mIsShooting = true; }
	void StopFire()  { mIsShooting = false; }

protected:
	virtual void FireBullet() abstract;
	virtual void CreateBulletPool() abstract;
};