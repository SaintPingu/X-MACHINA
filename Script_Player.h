#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class MasterModel;
class Rigidbody;
class BulletShader;
#pragma endregion


#pragma region EnumClass
enum class PlayerType {
	Unspecified = 0,
	Tank,
	Airplane
};
#pragma endregion


#pragma region Class
class Script_Player : public Component {
	COMPONENT_ABSTRACT(Script_Player, Component)

protected:
	PlayerType		mPlayerType{};
	GameObject*		mPlayer{};
	sptr<Rigidbody> mRigid{};
	Vec4x4			mSpawnTransform{ Matrix4x4::Identity() };

	int		mScore{};
	float	mMaxHP{};
	float	mHP{};
	float	mAcc{};

public:
	PlayerType GetPlayerType() const { return mPlayerType; }

	void SetHP(float hp) { mMaxHP = hp; mHP = hp; }
	void SetAcc(float acc) { mAcc = acc; }
	void SetSpawn(const Vec3& pos);
	void SetMaxSpeed(float speed);

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void ProcessInput() {}
	virtual void Move(DWORD dwDirection);
	virtual void Rotate(float x, float y, float z);

	void Respawn();
	void Explode();
	void Hit(float damage);
	void AddScore(int score);
};





class Script_ShootingPlayer : public Script_Player {
	COMPONENT_ABSTRACT(Script_ShootingPlayer, Script_Player)

protected:
	sptr<BulletShader>	mBulletShader{};
	float				mBulletSpeed{ 1.f };

private:
	float m_fFireDelay{};
	float m_fCurrFireDelay{};
	bool  mIsShooting{};
	float mbulletLifeTime{ 10.f };

public:
	void SetDamage(float damage);
	void SetFireDelay(float fDelay) { m_fFireDelay = fDelay; }

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void ProcessInput() override;

	virtual void FireBullet() abstract;
	virtual void RenderBullets() const;

	void StartFire() { mIsShooting = true; }
	void StopFire() { mIsShooting = false; }

	const std::list<sptr<GameObject>>* GetBullets() const;

	void CreateBullets(rsptr<const MasterModel> bulletModel);
};





class Script_AirplanePlayer : public Script_ShootingPlayer {
	COMPONENT(Script_AirplanePlayer, Script_ShootingPlayer)

private:
	Transform* mGunFirePos{};
	float mRotationSpeed{};

public:
	virtual void SetRotationSpeed(float speed) { mRotationSpeed = speed; }

public:
	virtual void Start() override;
	virtual void Update() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	virtual void ProcessInput() override;

	virtual void FireBullet() override;
	virtual void Rotate(DWORD rotationDir, float angle);
};





class Script_TankPlayer : public Script_ShootingPlayer {
	COMPONENT(Script_TankPlayer, Script_ShootingPlayer)

public:
	GameObject* mTurret{};
	GameObject* mGun{};
	Transform*  mShellFirePos{};

private:
	float mPrevTurretYaw{};
	float mRotationSpeed{};

public:
	float GetGunPitch() const;

	virtual void SetRotationSpeed(float speed) { mRotationSpeed = speed; }

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void FireBullet() override;
	virtual void Rotate(DWORD rotationDir, float angle);

	void RotateTurret(float pitch, float yaw);
	void RotateGun(float pitch);

private:
	void FixTurret();
};
#pragma endregion