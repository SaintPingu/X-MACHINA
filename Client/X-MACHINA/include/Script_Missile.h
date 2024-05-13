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
	sptr<Script_MainCamera> mCamera{};
	sptr<SphereCollider> mExplosionCollider{};
	float mExplosionDamage;

public:
	void SetExplosionDamage(float damage) { mExplosionDamage = damage; }

public:
	virtual void Init() override;
	virtual void Explode() override;

protected:
	virtual void StartFire() override;
};
#pragma endregion
