#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class Rigidbody;
#pragma endregion


#pragma region Class
// �Ѿ� ��ü
// Fire �Լ��� ���� active�Ѵ�.
class Script_Bullet : public Component {
	COMPONENT(Script_Bullet, Component)

private:
	GameObject*		mGameObject{};	// self GameObject
	sptr<Rigidbody> mRigid{};
	const Object*	mOwner{};		// �Ѿ��� �߻��� ��ü (�ڽ��� �浹���� �ʵ��� �Ѵ�)

	float mDamage{};				// damage of bullet
	float mMaxLifeTime{ 1000.f };	// �Ѿ� ����(�߻� �� �ִ� �����ð�)
	float mCurrLifeTime{};			// ���� ����

public:
	float GetDamage() { return mDamage; }

	void SetDamage(float damage) { mDamage = damage; }
	void SetOwner(const Object* object) { mOwner = object; }
	void SetLifeTime(float lifeTIme) { mMaxLifeTime = lifeTIme; }

public:
	virtual void Start() override;
	virtual void Update() override;

	virtual void OnCollisionStay(Object& other) override;

public:
	// [pos] ��ġ�� �����ϰ� [dir, up]�� ���� look ������ �����ϰ�, look �������� [speed]�� �ӵ��� �̵��ϵ��� �Ѵ�.
	void Fire(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed);
	// �Ѿ� ��ü�� �Ͷ߸���. (���� ó��)
	void Explode();

private:
	bool IsOwner(const Object* object) { return mOwner == object; }
	void Reset();
	// terrain�� �浹 ���θ� �˻��Ѵ�.
	bool IntersectTerrain();
};
#pragma endregion
