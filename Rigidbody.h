#pragma once
#include "Component.h"

enum class ForceMode { Accleration = 0, Impulse };

class Rigidbody : public Component {
	COMPONENT(Rigidbody, Component)

private	:
	bool mUseGravity	{ false };
	float mGravityScale	{ 1.f };

	float mMass		{ 1.f };
	float mDrag		{ 1.f };
	float mFriction	{ 1.f };
	float mMaxSpeed	{ FLT_MAX };
	Vec3 mVelocity{};

public:
	const Vec3& GetVelocity() const { return mVelocity; }

	void SetMass(float mass) { mMass = mass; }
	void SetFriction(float fFriction) { mFriction = fFriction; }
	void SetVelocity(const Vec3& xmf3Velocity) { mVelocity = xmf3Velocity; }
	void SetVelocity(float speed);
	void SetMaxSpeed(float speed) { mMaxSpeed = speed; }
	void SetGravity(bool useGravity) { mUseGravity = useGravity; }
	void SetGravityScale(float scale) { mGravityScale = scale; }
	void SetDrag(float drag) { mDrag = drag; }

public:
	virtual void Start() override;
	virtual void Update() override;

	virtual void OnCollisionStay(Object& other) override;

	void Stop();
	void AddForce(const Vec3& force, ForceMode forceMode = ForceMode::Accleration);
	void AddForce(const Vec3& dir, float speed, ForceMode forceMode = ForceMode::Accleration);
};