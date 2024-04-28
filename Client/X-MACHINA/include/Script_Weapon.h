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
class ParticleSystem;
#pragma endregion


enum class WeaponType {
	HandedGun,
	AssaultRifle,
	LightingGun,
	GatlinGun,
	ShotGun,
	MissileLauncher,
	Sniper,
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

protected:
	//std::vector<sptr<ParticleSystem>> mMuzzlePSs{};
	//std::vector<sptr<ParticleSystem>> mBulletPSs{};
	std::array<std::vector<std::string>, BulletPSTypeCount> mPSNames{};

	float mMaxFireDelay{};		// 총알 발사 딜레이
	float mCurFireDelay{};		// 현재 발사 딜레이

	float mMaxReloadTime{};		// 장전 시간
	float mCurReloadTime{};		// 현재 장전 시간

	float mMaxDistance{};		// 최대 사거리

	int mMaxMag{};				// 최대 장탄 수
	int mCurMag{};				// 현재 장탄 수
	int mBulletCntPerMag{};		// 탄창 단 총알 수
	int mCurBulletCnt{};		// 현재 발수 수
	int mBulletCntPerShot{ 1 };	// 발사 당 총알 수

	bool  mIsBoltAction{};		// 볼트액션 중인가?
	bool  mIsShooting{};		// 발사 버튼을 누르고 있는가?
	bool  mIsBeforeShooting{};	// 이전에 총알이 발사되었는가?

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual WeaponType GetWeaponType() const abstract;
	Transform* GetMuzzle() const { return mMuzzle; }
	float GetReloadTime() const { return mMaxReloadTime; }
	float GetFireDelay() const { return mMaxFireDelay; }
	bool IsReloading() const { return mIsReload || mIsBoltAction; }
	bool IsShooting() const { return mIsShooting; }

	void SetOwner(Script_GroundPlayer* owner) { mOwner = owner; }

public:
	void StartFire() { mIsShooting = true; }
	void StopFire() { mIsShooting = false; mIsBeforeShooting = false; }
	bool CheckReload();
	virtual void EndReload();
	virtual void StopReload() { InitReload(); }

protected:
	virtual void FireBullet() abstract;

	bool CanFire() const { return mCurFireDelay >= mMaxFireDelay; }
	void SetFiringMode(FiringMode firingMode);

	static float CalcFireDelay(float rpm) {return (1.f / (rpm / 60.f)); }

	virtual bool InitReload();
	virtual void StartReload();

	virtual float GetBulletSpeed() abstract;

private:

	void Update_Auto();
	void Update_SemiAuto();
	void Update_BoltAction();

	void Fire();

	virtual void CreateBulletPool() abstract;
	virtual void InitValues() abstract;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const abstract;
	virtual void SetParticleSystemNames() abstract;
};


class Script_BulletWeapon abstract : public Script_Weapon {
	COMPONENT_ABSTRACT(Script_BulletWeapon, Script_Weapon)

public:
	enum class BulletType { Bullet, Missile };

protected:
	Vec2 mErrX{};	// 좌우 오차
	Vec2 mErrY{};	// 상하 오차
	float mSpeerErr{};	// 속도 오차

protected:
	virtual void FireBullet() override;

	void InitBullet(rsptr<InstObject> bullet, float damage, float speed, BulletType bulletType = BulletType::Bullet) const;

private:
	virtual void CreateBulletPool() override;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const abstract;
};