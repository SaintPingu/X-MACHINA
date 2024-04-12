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
	Transform* mMuzzle{};		// �ѱ�
	sptr<ObjectPool> mBulletPool{};

	float mBulletDamage{};		// �Ѿ� �����
	float mBulletSpeed{};		// �Ѿ� �ӵ�

	float mMaxFireDelay{};		// �Ѿ� �߻� ������
	float mCurFireDelay{};		// ���� �߻� ������

	float mMaxReloadTime{};		// ���� �ð�
	float mCurReloadTime{};		// ���� ���� �ð�

	float mMaxDistance{};		// �ִ� ��Ÿ�

	int mMaxMag{};				// �ִ� ��ź ��
	int mCurMag{};				// ���� ��ź ��

	bool  mIsShooting{};		// �߻� ��ư�� ������ �ִ°�?

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