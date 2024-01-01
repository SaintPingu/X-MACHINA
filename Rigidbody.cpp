#include "stdafx.h"
#include "Rigidbody.h"
#include "Timer.h"


void Rigidbody::SetVelocity(float speed)
{
	mVelocity = Vector3::Resize(mVelocity, speed);
}


void Rigidbody::Start()
{

}

void Rigidbody::Update()
{
	const float gravity = Math::kGravity * mGravityScale;

	if (mFriction > FLT_EPSILON) {
		float normalForce{ mMass };
		if (mUseGravity) {
			normalForce *= gravity;
		}
		float friction = mFriction * normalForce;

		Vec3 frictionDir = Vector3::Negative(mVelocity);
		float mag = Vector3::Length(mVelocity);
		if (mag > FLT_EPSILON) {
			frictionDir = Vector3::Divide(frictionDir, mag);

			const Vec3 frictionForce = Vector3::Multiply(frictionDir, friction);
			const Vec3 dragForce = Vector3::Multiply(mVelocity, -mDrag);
			const Vec3 dragAcc = Vector3::Divide(dragForce, mMass);

			Vec3 frictionAcc = Vector3::Add(Vector3::Divide(frictionForce, mMass), dragAcc);

			Vec3 resultVec = Vector3::Add(mVelocity, Vector3::Multiply(frictionAcc, DeltaTime()));

			mVelocity.x = (mVelocity.x * resultVec.x < 0) ? 0.f : resultVec.x;
			mVelocity.y = (mVelocity.y * resultVec.y < 0) ? 0.f : resultVec.y;
			mVelocity.z = (mVelocity.z * resultVec.z < 0) ? 0.f : resultVec.z;

			if (mUseGravity) {
				const Vec3 gravityForce = Vector3::Multiply(Vector3::Down(), normalForce);
				const Vec3 gravityAcc = Vector3::Divide(gravityForce, mMass);
				mVelocity = Vector3::Add(mVelocity, Vector3::Multiply(gravityAcc, DeltaTime()));
			}
		}
	}


	mObject->Translate(Vector3::Multiply(mVelocity, DeltaTime()));
}


void Rigidbody::OnCollisionStay(Object& other)
{
	// 面倒贸府 内靛 鞘夸...
}



void Rigidbody::Stop()
{
	SetVelocity(Vector3::Zero());
}

void Rigidbody::AddForce(const Vec3& force, ForceMode forceMode)
{
	float t{ 1 };
	if (forceMode == ForceMode::Accleration) {
		t = DeltaTime();
	}

	const Vec3 acc = Vector3::Multiply(Vector3::Divide(Vector3::Multiply(force, mAcc), mMass), t);
	mVelocity = Vector3::Add(mVelocity, acc);
	if (Vector3::Length(mVelocity) > mMaxSpeed) {
		SetVelocity(mMaxSpeed);
	}
}

void Rigidbody::AddForce(const Vec3& dir, float speed, ForceMode forceMode)
{
	AddForce(Vector3::Multiply(dir, speed), forceMode);
}