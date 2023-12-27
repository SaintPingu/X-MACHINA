#pragma once
#include "Component.h"


class Script_ExplosiveObject : public Component {
	COMPONENT(Component, Script_ExplosiveObject)

private:
	bool mIsExploded{};
	std::function<void(const Vec3&)> CreateFX{};

	float mHP{};

public:
	bool IsExploded() { return mIsExploded; };

	void SetFX(std::function<void(const Vec3&)> func) { CreateFX = func; }
	void SetHP(float hp) { mHP = hp; }

	void Explode();

	void Hit(float damage);

	virtual void OnCollisionStay(CObject& other) override;
};