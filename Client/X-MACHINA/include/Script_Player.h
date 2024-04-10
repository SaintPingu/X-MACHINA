#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class MasterModel;
class Rigidbody;
class InstObject;
class ObjectPool;
class Animator;
class AnimatorController;
class Weapon;
#pragma endregion


#pragma region EnumClass
// �÷��̾� ��ü Ÿ��
enum class PlayerType {
	Unspecified = 0,
	Tank,
	Airplane,
	Human
};

class Movement : public DwordOverloader<Movement> {
	DWORD_OVERLOADER(Movement)

	static const DWORD None   = 0x00;
	static const DWORD Stand  = 0x01;
	static const DWORD Sit    = 0x02;
	static const DWORD Walk   = 0x10;
	static const DWORD Run    = 0x20;
	static const DWORD Sprint = 0x40;
};
#pragma endregion


#pragma region Class
class Script_Player : public Component {
	COMPONENT_ABSTRACT(Script_Player, Component)

protected:
	PlayerType		mPlayerType{};
	GameObject*		mPlayer{};		// self GameObject
	Matrix			mSpawnTransform{};	// ������ ����

	int		mScore{};
	float	mMaxHP{};
	float	mHP{};

public:
	PlayerType GetPlayerType() const { return mPlayerType; }

	void SetHP(float hp) { mMaxHP = hp; mHP = hp; }
	// player�� [pos]�� ��ġ��Ű�� �ش� ��ġ�� ������ �������� �����Ѵ�.
	void SetSpawn(const Vec3& pos);

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void ProcessInput() {}
	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam) {}

	virtual void Rotate(float pitch, float yaw, float roll);

	void Respawn();
	void Explode();
	void Hit(float damage);
	void AddScore(int score);
};





// �Ѿ��� �߻��� �� �ִ� �÷��̾�
class Script_ShootingPlayer : public Script_Player {
	COMPONENT_ABSTRACT(Script_ShootingPlayer, Script_Player)

protected:
	sptr<ObjectPool> mBulletPool{};

private:
	float mBulletDamage{};
	float mBulletSpeed{};
	float mMaxFireDelay{};		// �Ѿ� �߻� ������
	float mCrntFireDelay{};		// ���� �߻� ������
	bool  mIsShooting{};		// �߻����ΰ�?

public:
	float GetBulletDamage() const { return mBulletDamage; }
	float GetBulletSpeed()  const { return mBulletSpeed; }

	void SetBulletDamage(float damage) { mBulletDamage = damage; }
	void SetBulletSpeed(float speed) { mBulletSpeed = speed; }
	void SetFireDelay(float fDelay) { mMaxFireDelay = fDelay; }

public:
	virtual void Update() override;

	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;

private:
	void StartFire() { mIsShooting = true; }
	void StopFire() { mIsShooting = false; }

	virtual void FireBullet() abstract;
};




// ���� �÷��̾�
class Script_GroundPlayer : public Script_ShootingPlayer {
	COMPONENT(Script_GroundPlayer, Script_ShootingPlayer)


private:
	const float mkSitWalkSpeed   = 1.5f;
	const float mkStandWalkSpeed = 2.2f;
	const float mkRunSpeed       = 5.f;
	const float mkSprintSpeed    = 8.f;

	sptr<ObjectPool> mBulletPool{};

	Movement mPrevMovement{};

	bool mIsAim{};
	float mParamV{}, mParamH{};

	sptr<Animator> mAnimator{};
	sptr<AnimatorController> mController{};

	float mMovementSpeed{};
	float mRotationSpeed{};

	sptr<GameObject> mWeapon{};
	std::vector<sptr<GameObject>> mWeapons{};
	Transform* mSpineBone{};
	Transform* mFirePos{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	void UpdateParams(Dir dir, float v, float h);
	virtual void ProcessInput() override;

	// direction �������� �̵��Ѵ�.
	virtual void Move(Dir dir);
	// [dir]������ �ٶ󺸵��� ȸ���Ѵ�.
	void RotateTo(Dir dir);

	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam) override;

private:
	virtual void FireBullet() override;

	void SetWeapon(int weaponIdx);
	void UpdateParam(float val, float& param);
};

#pragma endregion