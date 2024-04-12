#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Transform;
class InstObject;
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

public:
	enum class FiringMode { SemiAuto, Auto, BoltAction };

private:
	std::function<void()> updateFunc{ std::bind(&Script_Weapon::Update_Auto, this) };

protected:
	Transform* mMuzzle{};		// �ѱ�
	sptr<ObjectPool> mBulletPool{};

	float mMaxFireDelay{};		// �Ѿ� �߻� ������
	float mCurFireDelay{};		// ���� �߻� ������

	float mMaxReloadTime{};		// ���� �ð�
	float mCurReloadTime{};		// ���� ���� �ð�

	float mMaxDistance{};		// �ִ� ��Ÿ�

	int mMaxMag{};				// �ִ� ��ź ��
	int mCurMag{};				// ���� ��ź ��

	bool  mIsShooting{};		// �߻� ��ư�� ������ �ִ°�?
	bool  mIsBeforeShooting{};	// ������ �Ѿ��� �߻�Ǿ��°�?

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	Transform* GetMuzzle() const { return mMuzzle; }

public:
	void StartFire() { mIsShooting = true; }
	void StopFire() { mIsShooting = false; mIsBeforeShooting = false; }

protected:
	virtual void CreateBulletPool() abstract;
	virtual void FireBullet() abstract;

	bool CanFire() const { return mCurFireDelay >= mMaxFireDelay; }
	void SetFiringMode(FiringMode firingMode);

	static float CalcFireDelay(float rpm) {return (1.f / (rpm / 60.f)); }

private:
	void Update_SemiAuto();
	void Update_Auto();

	virtual void InitValues() abstract;
	virtual void BulletInitFunc(rsptr<InstObject> bullet) const abstract;
};