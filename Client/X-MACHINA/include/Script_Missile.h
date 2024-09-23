#pragma once


#pragma region Include
#include "Script_ExplosiveBullet.h"
#pragma endregion


#pragma region ClassForwardDecl
class SphereCollider;
class Script_MainCamera;
class ParticleSystem;
#pragma endregion


#pragma region Class
class Script_Missile : public Script_ExplosiveBullet {
	COMPONENT(Script_Missile, Script_ExplosiveBullet)

private:
	ParticleSystem* mContrail{};

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	virtual void Fire(const Vec3& pos, const Vec3& dir) override;
	void Fire(const Vec3& pos, const Vec3& dir, const Vec2& err);

	virtual void Explode() override;
};

class Script_DeusMissile : public Script_Missile {
	COMPONENT(Script_DeusMissile, Script_Missile)

public:
	virtual void Awake() override;
};

#pragma endregion
