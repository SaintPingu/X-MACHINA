#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Transform;
class InstObject;
class GridObject;
class ObjectPool;
class Script_GroundPlayer;
#pragma endregion


enum class WeaponType {
	HandedGun,
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

public:
	enum class FiringMode { SemiAuto, Auto, BoltAction };

private:
	std::function<void()> updateFunc{ std::bind(&Script_Weapon::Update_Auto, this) };
	bool mIsReload{};
	Script_GroundPlayer* mOwner{};

protected:
	Transform* mMuzzle{};		// 총구
	sptr<ObjectPool> mBulletPool{};

	float mMaxFireDelay{};		// 총알 발사 딜레이
	float mCurFireDelay{};		// 현재 발사 딜레이

	float mMaxReloadTime{};		// 장전 시간
	float mCurReloadTime{};		// 현재 장전 시간

	float mMaxDistance{};		// 최대 사거리

	int mMaxMag{};				// 최대 장탄 수
	int mCurMag{};				// 현재 장탄 수

	bool  mIsShooting{};		// 발사 버튼을 누르고 있는가?
	bool  mIsBeforeShooting{};	// 이전에 총알이 발사되었는가?

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual WeaponType GetWeaponType() const abstract;
	Transform* GetMuzzle() const { return mMuzzle; }
	float GetReloadTime() const { return mMaxReloadTime ; }
	bool IsReloading() const { return mIsReload; }

	void SetOwner(Script_GroundPlayer* owner) { mOwner = owner; }

public:
	void StartFire() { mIsShooting = true; }
	void StopFire() { mIsShooting = false; mIsBeforeShooting = false; }
	virtual void InitReload();
	virtual void EndReload();
	virtual void StopReload() { InitReload(); }

protected:
	virtual void FireBullet() abstract;

	bool CanFire() const { return mCurFireDelay >= mMaxFireDelay; }
	void SetFiringMode(FiringMode firingMode);

	static float CalcFireDelay(float rpm) {return (1.f / (rpm / 60.f)); }

	virtual void StartReload();

private:
	void Update_SemiAuto();
	void Update_Auto();

	void Fire();

	virtual void CreateBulletPool() abstract;
	virtual void InitValues() abstract;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const abstract;
};


class Script_BulletWeapon abstract : public Script_Weapon {
	COMPONENT_ABSTRACT(Script_BulletWeapon, Script_Weapon)

protected:
	virtual void FireBullet() override;

	void InitBullet(rsptr<InstObject> bullet, float damage, float speed) const;

private:
	virtual void CreateBulletPool() override final;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const abstract;
};