#pragma once

#pragma region Include
#include "Component/Component.h"
#include "Script_Network.h"
#pragma endregion



#pragma region ClassForwardDecl
class Transform;
class InstObject;
class GridObject;
class ObjectPool;
class Script_GroundPlayer;
class ParticleSystem;
#pragma endregion

class Script_NetworkWeapon : public Script_Network {
	COMPONENT(Script_NetworkWeapon, Script_Network)

public:
	enum class FiringMode { SemiAuto, Auto, BoltAction };

private:
	std::function<void()> updateFunc{ std::bind(&Script_NetworkWeapon::Update_Auto, this) };
	bool mIsReload{};
	Script_GroundPlayer* mOwner{};

protected:
	Transform* mMuzzle{};		// �ѱ�
	sptr<ObjectPool> mBulletPool{};

protected:
	std::array<std::vector<std::string>, BulletPSTypeCount> mPSNames{};

	float mMaxFireDelay{};		// �Ѿ� �߻� ������
	float mCurFireDelay{};		// ���� �߻� ������

	float mMaxReloadTime{};		// ���� �ð�
	float mCurReloadTime{};		// ���� ���� �ð�

	float mMaxDistance{};		// �ִ� ��Ÿ�

	int mMaxMag{};				// �ִ� ��ź ��
	int mCurMag{};				// ���� ��ź ��
	int mBulletCntPerMag{};		// źâ �� �Ѿ� ��
	int mCurBulletCnt{};		// ���� �߼� ��
	int mBulletCntPerShot{ 1 };	// �߻� �� �Ѿ� ��

	bool  mIsBoltAction{};		// ��Ʈ�׼� ���ΰ�?
	bool  mIsShooting{};		// �߻� ��ư�� ������ �ִ°�?
	bool  mIsBeforeShooting{};	// ������ �Ѿ��� �߻�Ǿ��°�?

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual WeaponType GetWeaponType() const abstract;
	virtual WeaponName GetWeaponName() const abstract;
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

	static float CalcFireDelay(float rpm) { return (1.f / (rpm / 60.f)); }

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

public:
	static std::string GetWeaponModelName(WeaponName weaponName);
	static int GetWeaponIdx(WeaponType weaponType);

};

