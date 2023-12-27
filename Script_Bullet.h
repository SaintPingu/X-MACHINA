#pragma once
#include "Component.h"

class GameObject;
class Rigidbody;

class Script_Bullet : public Component {
	COMPONENT(Component, Script_Bullet)

private:
	GameObject* mGameObject{};
	sptr<Rigidbody> mRigid{};
	const Object* mOwner{};

	float mDamage{};
	float mLifeTime{ 1000.0f };
	float mElapsedTimeAfterFire{};

	bool IsOwner(const Object* object);
	void Reset();
	bool IntersectTerrain();

public:
	virtual void Start() override;
	virtual void Update() override;

	virtual void OnCollisionStay(Object& other) override;

	float GetDamage() { return mDamage; }

	void SetDamage(float damage) { mDamage = damage; }
	void SetOwner(const Object* object) { mOwner = object; }
	void SetLifeTime(float lifeTIme) { mLifeTime = lifeTIme; }

	void Fire(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed);
	void Explode();
};