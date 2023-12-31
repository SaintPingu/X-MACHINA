#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class Rigidbody;
#pragma endregion


#pragma region Class
class Script_Bullet : public Component {
	COMPONENT(Script_Bullet, Component)

private:
	GameObject*		mGameObject{};
	sptr<Rigidbody> mRigid{};
	const Object*	mOwner{};

	float mDamage{};
	float mMaxLifeTime{ 1000.f };
	float mCurrLifeTime{};

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
	void Fire(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed);
	void Explode();

private:
	bool IsOwner(const Object* object) { return mOwner == object; }
	void Reset();
	bool IntersectTerrain();
};
#pragma endregion
