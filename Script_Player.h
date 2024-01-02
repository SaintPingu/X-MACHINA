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
// �÷��̾� ��ü Ÿ��
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
	GameObject*		mPlayer{};		// self GameObject
	sptr<Rigidbody> mRigid{};
	Vec4x4			mSpawnTransform{ Matrix4x4::Identity() };	// ������ ����

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
	// direction �������� �̵��Ѵ�.
	virtual void Move(DWORD dwDirection);
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
	sptr<BulletShader>	mBulletShader{};
	float				mBulletSpeed{ 1.f };

private:
	float mMaxFireDelay{};		// �Ѿ� �߻� ������
	float mCurrFireDelay{};		// ���� �߻� ������
	bool  mIsShooting{};		// �߻����ΰ�?

public:
	void SetDamage(float damage);
	void SetFireDelay(float fDelay) { mMaxFireDelay = fDelay; }

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void ProcessInput() override;

	virtual void FireBullet() abstract;
	virtual void RenderBullets() const;

	void StartFire() { mIsShooting = true; }
	void StopFire() { mIsShooting = false; }

	// [bulletModel]�� ���� BulletShader�� �����Ѵ�.
	void CreateBullets(rsptr<const MasterModel> bulletModel);
};





// �︮���� �÷��̾�
class Script_AirplanePlayer : public Script_ShootingPlayer {
	COMPONENT(Script_AirplanePlayer, Script_ShootingPlayer)

private:
	Transform* mGunFirePos{};	// �Ѿ� �߻� ��ġ
	float mRotationSpeed{};		// ȸ�� �ӵ�

public:
	virtual void SetRotationSpeed(float speed) { mRotationSpeed = speed; }

public:
	virtual void Start() override;
	virtual void Update() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	virtual void ProcessInput() override;

	virtual void FireBullet() override;
	// [rotationDir]�������� [angle]��ŭ ȸ���Ѵ�.
	virtual void Rotate(DWORD rotationDir, float angle);
};
#pragma endregion