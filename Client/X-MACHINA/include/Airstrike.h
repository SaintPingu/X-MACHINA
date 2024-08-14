#pragma once

#pragma region ClassForwardDecl
class ObjectPool;
class InstObject;
#pragma endregion


class Airstrike {
private:
	std::string mFireSound{};
	sptr<ObjectPool> mMissilePool{};
	float mExpDamage{};
	float mMissileSpeed{};
	int mMaxMissileCnt{};
	int mCntPerFire{};
	int mRemainMissileCnt{};

	float mCurStrikeDelay{};
	float mMaxStrikeDelay{};
	float mCurFireDelay{};
	float mMaxFireDelay{};
	Vec3 mFirePos{};
	Vec3 mFireDir{};
	int mCurFireCnt{};
	bool mIsFired{};

	Vec2 mErrX;
	Vec2 mErrY;

	std::array<std::vector<std::string>, BulletPSTypeCount> mPSNames{};

public:
	void SetFireSound(const std::string& sound) { mFireSound = sound; }

public:
	void Init();
	void Update();
	void On();

	void StartFire(const Vec3& pos, const Vec3& dir);
	void Fire(const Vec3& pos, const Vec3& dir); // for remote player
	void SetPlayerBullet();

private:
	void CreateMissilePool();
	void MissileInitFunc(rsptr<InstObject> missile) const;
	void SetParticleSystemNames();

	void Fire();
};
