#pragma once
#include "Component.h"

class CGameObject;
class Rigidbody;

class Script_Bullet : public Component {
	COMPONENT(Component, Script_Bullet)

private:
	CGameObject* mGameObject{};
	sptr<Rigidbody> mRigid{};
	const CObject* mOwner{};

	float mDamage{};
	float mLifeTime{ 1000.0f };
	float mElapsedTimeAfterFire{};

	bool IsOwner(const CObject* object);
	void Reset();
	bool IntersectTerrain();

public:
	virtual void Start() override;
	virtual void Update() override;

	virtual void OnCollisionStay(CObject& other) override;

	float GetDamage() { return mDamage; }

	void SetDamage(float damage) { mDamage = damage; }
	void SetOwner(const CObject* object) { mOwner = object; }
	void SetLifeTime(float lifeTIme) { mLifeTime = lifeTIme; }

	void Fire(const Vec3& pos, const Vec3& dir, const Vec3& up, float speed);
	void Explode();
};