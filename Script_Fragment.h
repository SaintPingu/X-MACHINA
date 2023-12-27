#pragma once
#include "Component.h"

class Rigidbody;

class Script_Fragment : public Component {
	COMPONENT(Component, Script_Fragment)

private:
	sptr<Rigidbody> mRigid{};
	Vec3 mColor{};
	Vec3 mRotationAxis = Vector3::Up();
	Vec3 mMovingDir{};
	float mMovingSpeed = 0.f;
	float mRotationSpeed = 0.f;

public:
	virtual void Start() override;
	virtual void Update() override;

	void Active(const Vec3& pos);
	void SetColor(const Vec3& color) { mColor = color; }
	void SetRotationAxis(const Vec3& rotationAxis) { mRotationAxis = Vector3::Normalize(rotationAxis); }
	void SetRotationSpeed(float speed) { mRotationSpeed = speed; }
	void SetMovingSpeed(float speed) { mMovingSpeed = speed; }
	void SetMovingDir(const Vec3& dir) { mMovingDir = dir; }

	const Vec3& GetColor() const { return mColor; }

};