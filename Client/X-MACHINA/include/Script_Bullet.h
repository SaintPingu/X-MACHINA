#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class ParticleSystem;
#pragma endregion


#pragma region Class
// �Ѿ� ��ü
// Fire �Լ��� ���� active�Ѵ�.
class Script_Bullet : public Component {
	COMPONENT(Script_Bullet, Component)

private:
	GameObject*		mGameObject{};	// self GameObject
	const Object*	mOwner{};		// �Ѿ��� �߻��� ��ü (�ڽ��� �浹���� �ʵ��� �Ѵ�)

	std::array<std::vector<sptr<ParticleSystem>>, BulletPSTypeCount> mPSs;

	float mSpeed{};					// speed of bullet
	float mDamage{};				// damage of bullet
	float mMaxLifeTime{ 2.f };		// �Ѿ� ����(�߻� �� �ִ� �����ð�)
	float mCurrLifeTime{};			// ���� ����
	bool  mIsSetPSs{};

public:
	float GetDamage() { return mDamage; }
	float IsSetPSs() { return mIsSetPSs; }

	void SetDamage(float damage) { mDamage = damage; }
	void SetSpeed(float speed) { mSpeed = speed; }
	void SetOwner(const Object* object) { mOwner = object; }
	void SetLifeTime(float lifeTIme) { mMaxLifeTime = lifeTIme; }
	void SetParticleSystems(BulletPSType type, const std::vector<std::string>& psNames);

public:
	virtual void Awake() override;
	virtual void Update() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	// [pos] ��ġ�� �����ϰ� [dir, up]�� ���� look ������ �����ϰ�, look �������� [speed]�� �ӵ��� �̵��ϵ��� �Ѵ�.
	void Fire(const Vec3& pos, const Vec3& dir, const Vec3& up);
	void Fire(const Transform& transform);

	// �Ѿ� ��ü�� �Ͷ߸���. (���� ó��)
	void Explode();
	void PlayPSs(BulletPSType type); 
	void StopPSs(BulletPSType type); 
	void ResetPSs(BulletPSType type); 

private:
	bool IsOwner(const Object* object) { return mOwner == object; }
	void Reset();
	// terrain�� �浹 ���θ� �˻��Ѵ�.
	bool IntersectTerrain();
};
#pragma endregion
