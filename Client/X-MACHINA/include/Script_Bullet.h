#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class InstObject;
class Rigidbody;
class ParticleSystem;
#pragma endregion


#pragma region Class
class Script_Bullet : public Component {
	COMPONENT(Script_Bullet, Component)

private:
	InstObject*		mGameObject{};	// self GameObject
	const Object*	mOwner{};		// �Ѿ��� �߻��� ��ü (�ڽ��� �浹���� �ʵ��� �Ѵ�)
	sptr<ParticleSystem> mParticleSystem{};
	sptr<Rigidbody> mRigid{};

	std::array<std::vector<sptr<ParticleSystem>>, BulletPSTypeCount> mPSs;

	float mSpeed{};					// speed of bullet
	float mDamage{};				// damage of bullet
	float mMaxLifeTime{ 2.f };		// �Ѿ� ����(�߻� �� �ִ� �����ð�)
	float mCurrLifeTime{};			// ���� ����

public:
	float GetDamage() const { return mDamage; }

	void SetDamage(float damage) { mDamage = damage; }
	void SetSpeed(float speed) { mSpeed = speed; }
	void SetOwner(const Object* object) { mOwner = object; }
	void SetLifeTime(float lifeTIme) { mMaxLifeTime = lifeTIme; }
	void SetParticleSystems(BulletPSType type, const std::vector<std::string>& psNames);

public:
	virtual void Update() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	virtual void Init();

	// [pos] ��ġ�� �����ϰ� [dir, up]�� ���� look ������ �����ϰ�, look �������� [speed]�� �ӵ��� �̵��ϵ��� �Ѵ�.
	void Fire(const Vec3& pos, const Vec3& dir, const Vec3& up);
	// [err] ��ŭ ������ ź�� ������.
	void Fire(const Transform& transform, const Vec2& err = Vector2::Zero);

	// �Ѿ� ��ü�� �Ͷ߸���. (���� ó��)
	virtual void Explode();
	void PlayPSs(BulletPSType type); 
	void StopPSs(BulletPSType type); 
	void ResetPSs(BulletPSType type); 

protected:
	virtual void StartFire() {};

private:
	bool IsOwner(const Object* object) { return mOwner == object; }
	void Reset();
	// terrain�� �浹 ���θ� �˻��Ѵ�.
	bool IntersectTerrain();
};
#pragma endregion
