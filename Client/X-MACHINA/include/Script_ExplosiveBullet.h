#pragma once

#pragma region Include
#include "Script_Bullet.h"
#pragma endregion


#pragma region ClassForwardDecl
class SphereCollider;
class Script_MainCamera;
#pragma endregion


#pragma region Class
class Script_ExplosiveBullet abstract : public Script_Bullet {
	COMPONENT_ABSTRACT(Script_ExplosiveBullet, Script_Bullet)

private:
	ObjectTag mExplosionTag{};
	sptr<SphereCollider> mExplosionCollider{};
	float mExplosionDamage{};

public:
	virtual void OnCollisionEnter(Object& other) override;

public:
	void SetExplosionTag(ObjectTag tag) { mExplosionTag = tag; }
	void SetExplosionDamage(float damage) { mExplosionDamage = damage; }

public:
	virtual void Init() override;
	virtual void Explode() override;

protected:
	virtual void StartFire() override;
};
#pragma endregion