#pragma once


#pragma region Include
#include "Script_Bullet.h"
#pragma endregion


#pragma region ClassForwardDecl
class SphereCollider;
class Script_MainCamera;
#pragma endregion


#pragma region Class
class Script_Missile : public Script_Bullet {
	COMPONENT(Script_Missile, Script_Bullet)

private:
	const Object* mOwner{};
	sptr<Script_MainCamera> mCamera{};
	sptr<SphereCollider> mExplosionCollider{};
	float mExplosionDamage;

public:
	virtual void OnCollisionEnter(Object& other) override;

public:
	bool IsOwner(const Object* object) { return mOwner == object; }

	void SetExplosionDamage(float damage) { mExplosionDamage = damage; }

	void SetOwner(const Object* owner) { mOwner = owner; }

public:
	virtual void Fire(const Vec3& pos, const Vec3& dir, const Vec3& up) override;

	virtual void Init() override;
	virtual void Explode() override;

protected:
	virtual void StartFire() override;
};
#pragma endregion
