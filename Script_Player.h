#pragma once
#include "Component.h"

class CGameObject;
class Camera;
class CMasterModel;
class Rigidbody;

enum class PlayerType { Unspecified = 0, Tank, Airplane };






class Script_Player : public Component {
	COMPONENT_ABSTRACT(Component, Script_Player)

protected:
	CGameObject* mPlayer{};
	sptr<Rigidbody> mRigid{};
	Vec4x4 mSpawnTransform{ Matrix4x4::Identity() };

	int mScore{};
	float mMaxHP{};
	float mHP{};
	float mAcc{};

public:
	PlayerType					mPlayerType{};

public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Getter ] /////

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Setter ] /////

	void SetSpawn(const Vec3& pos);
	void SetAcc(float acc);
	void SetMaxSpeed(float speed);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// [ Others ] /////

	virtual void Start() override;
	virtual void Update() override;
	virtual void ProcessInput() {}

	virtual void Move(DWORD dwDirection);

	virtual void Rotate(float x, float y, float z);

	void Respawn();
	void Explode();
	void Hit(float damage);
	void SetHP(float hp) { mMaxHP = hp; mHP = hp; }
	void AddScore(int score);
};



class CBulletShader;

class Script_ShootingPlayer : public Script_Player {
	COMPONENT_ABSTRACT(Script_Player, Script_ShootingPlayer)

private:
	float m_fFireDelay{};
	float m_fCrntFireDelay{};
	bool mIsShooting{};
	float mbulletLifeTime{ 10.0f };

protected:
	sptr<CBulletShader> mBulletShader{};
	float mBulletSpeed{ 1.f };

public:

	virtual void Start() override;
	virtual void Update() override;
	virtual void ProcessInput() override;

	void SetDamage(float damage);

	void SetFireDelay(float fDelay) { m_fFireDelay = fDelay; }
	void StartFire() { mIsShooting = true; }
	void StopFire() { mIsShooting = false; }

	virtual void FireBullet() abstract;
	virtual void RenderBullets() const;

	const std::list<sptr<CGameObject>>* GetBullets() const;

	void CreateBullets(rsptr<const CMasterModel> bulletModel);
};





class Script_AirplanePlayer : public Script_ShootingPlayer {
	COMPONENT(Script_ShootingPlayer, Script_AirplanePlayer)

private:
	Transform* mGunFirePos{};
	float mRotationSpeed{};

public:
	virtual void Start() override;
	virtual void Update() override;
	virtual void ProcessInput() override;
	virtual void OnCollisionStay(CObject& other) override;

	virtual void Rotate(DWORD rotationDir, float angle);

	virtual void FireBullet();

	virtual void SetRotationSpeed(float speed) { mRotationSpeed = speed; }
};



class Script_TankPlayer : public Script_ShootingPlayer {
	COMPONENT(Script_ShootingPlayer, Script_TankPlayer)

private:
	float mPrevTurretYaw{};
	float mRotationSpeed{};

public:
	CGameObject* mTurret{};
	CGameObject* mGun{};
	Transform* mShellFirePos{};

private:
	void FixTurret();

public:

	virtual void Start() override;
	virtual void Update() override;

	virtual void SetRotationSpeed(float speed) { mRotationSpeed = speed; }

	virtual void Rotate(DWORD rotationDir, float angle);
	void RotateTurret(float pitch, float yaw);
	void RotateGun(float pitch);

	virtual void FireBullet();

	float GetGunPitch() const;
};