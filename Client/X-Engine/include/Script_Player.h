#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class MasterModel;
class Rigidbody;
class InstObject;
class ObjectPool;
class Animator;
class Weapon;
#pragma endregion


#pragma region EnumClass
// 플레이어 객체 타입
enum class PlayerType {
	Unspecified = 0,
	Tank,
	Airplane,
	Human
};
#pragma endregion


#pragma region Class
class Script_Player : public Component {
	COMPONENT_ABSTRACT(Script_Player, Component)

protected:
	PlayerType		mPlayerType{};
	GameObject*		mPlayer{};		// self GameObject
	sptr<Rigidbody> mRigid{};
	Vec4x4			mSpawnTransform{ Matrix4x4::Identity() };	// 리스폰 지점

	int		mScore{};
	float	mMaxHP{};
	float	mHP{};

public:
	PlayerType GetPlayerType() const { return mPlayerType; }

	void SetHP(float hp) { mMaxHP = hp; mHP = hp; }
	// player를 [pos]로 위치시키고 해당 위치를 리스폰 지점으로 설정한다.
	void SetSpawn(const Vec3& pos);

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void ProcessInput() {}
	// direction 방향으로 이동한다.
	virtual void Move(DWORD dwDirection);
	virtual void Rotate(float pitch, float yaw, float roll);

	void Respawn();
	void Explode();
	void Hit(float damage);
	void AddScore(int score);
};





// 총알을 발사할 수 있는 플레이어
class Script_ShootingPlayer : public Script_Player {
	COMPONENT_ABSTRACT(Script_ShootingPlayer, Script_Player)

protected:
	sptr<ObjectPool> mBulletPool{};

private:
	float mBulletDamage{};
	float mBulletSpeed{};
	float mMaxFireDelay{};		// 총알 발사 딜레이
	float mCurrFireDelay{};		// 현재 발사 딜레이
	bool  mIsShooting{};		// 발사중인가?

public:
	void SetBulletDamage(float damage) { mBulletDamage = damage; }
	void SetBulletSpeed(float speed) { mBulletSpeed = speed; }
	void SetFireDelay(float fDelay) { mMaxFireDelay = fDelay; }

public:
	virtual void Update() override;

public:
	virtual void ProcessInput() override;

	virtual void FireBullet() abstract;
	virtual void RenderBullets() const;

	void StartFire() { mIsShooting = true; }
	void StopFire() { mIsShooting = false; }

	// [bulletModel]에 대한 BulletShader를 생성한다.
	void CreateBullets(rsptr<const MasterModel> bulletModel);

protected:
	void FireBullet(const Vec3& pos, const Vec3& dir, const Vec3& up);

	void InitBullet(rsptr<InstObject> object);
};





// 헬리콥터 플레이어
class Script_AirplanePlayer : public Script_ShootingPlayer {
	COMPONENT(Script_AirplanePlayer, Script_ShootingPlayer)

private:
	Transform* mGunFirePos{};	// 총알 발사 위치
	float mRotationSpeed{};		// 회전 속도

public:
	virtual void SetRotationSpeed(float speed) { mRotationSpeed = speed; }

public:
	virtual void Start() override;
	virtual void Update() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	virtual void ProcessInput() override;

	virtual void FireBullet() override;
	// [rotationDir]방향으로 [angle]만큼 회전한다.
	virtual void Rotate(DWORD rotationDir, float angle);
};


// 지상 플레이어
class Script_GroundPlayer : public Script_ShootingPlayer {
	COMPONENT(Script_GroundPlayer, Script_ShootingPlayer)

private:
	float mParamV{}, mParamH{};

	sptr<Animator> mAnimator{};

	float mRotationSpeed{};
	sptr<Weapon> mWeapon{};
	std::vector<sptr<Weapon>> mWeapons{};
	Transform* mSpine{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	void UpdateParams(float v, float h);
	virtual void ProcessInput() override;

	virtual void FireBullet() override;
	// [rotationDir]방향으로 [angle]만큼 회전한다.
	virtual void Rotate(DWORD rotationDir, float angle);

	virtual void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	virtual void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);

private:
	void SetWeapon(int weaponIdx);
	void UpdateParam(float val, float& param);
};

#pragma endregion