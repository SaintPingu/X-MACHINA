#pragma once
#include "Component.h"


// ���� ������ ��ü (�÷��̾� ����)
class Script_ExplosiveObject : public Component {
	COMPONENT(Script_ExplosiveObject, Component)

private:
	bool mIsExploded{};

	float mHP{};		// current hp

public:
	bool IsExploded() { return mIsExploded; };

	void SetHP(float hp) { mHP = hp; }
	// set explosion effect function.
	void SetFX(std::function<void(const Vec3&)> func) { CreateFX = func; }

	virtual void OnCollisionStay(Object& other) override;

public:
	void Explode();

	void Hit(float damage);

private:
	std::function<void(const Vec3&)> CreateFX{};		// effect function
};