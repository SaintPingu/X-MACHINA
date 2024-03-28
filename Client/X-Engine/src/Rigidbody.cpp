#include "EnginePch.h"
#include "Component/Rigidbody.h"

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

	// 마찰력을 적용한다.
	if (mFriction > FLT_EPSILON) {
		float normalForce{ mMass };		// 수직항력
		if (mUseGravity) {
			normalForce *= gravity;		// 수직 항력에 중력을 적용한다.
		}
		
		const float friction = mFriction * normalForce;		// 마찰 계수 = 마찰 계수 * 수직항력

		const float mag = mVelocity.Length();		// 현재 속도 크기
		if (mag > FLT_EPSILON) {							// 현재 속도가 있다면
			const Vec3 frictionDir = -mVelocity / mag;	// 마찰력 방향(현재 속도의 반대방향)

			const Vec3 frictionForce = frictionDir * friction;			// 마찰력 = 마찰력 방향 * 마찰계수
			const Vec3 dragForce = mVelocity * -mDrag;					// 저항력 = 속도 * 저항 계수 * -1
			const Vec3 dragAcc = dragForce / mMass;							// 저항 가속도 = 저항력 * 질량

			const Vec3 frictionAcc = (frictionForce / mMass) + dragAcc;	// 마찰 가속도 = (마찰력/질량) + 저항 가속도

			const Vec3 resultVec = mVelocity + (frictionAcc * DeltaTime());	// 결과 = 현재 속도 + (마찰 가속도 * DeltaTime)

			// 각 성분의 방향이 바뀌면 0으로 조정한다.
			mVelocity.x = (mVelocity.x * resultVec.x < 0) ? 0.f : resultVec.x;
			mVelocity.y = (mVelocity.y * resultVec.y < 0) ? 0.f : resultVec.y;
			mVelocity.z = (mVelocity.z * resultVec.z < 0) ? 0.f : resultVec.z;

			// 중력이 활성화된 경우 중력 가속도를 계산해 속도를 재계산한다.
			if (mUseGravity) {
				const Vec3 gravityForce = Vec3::Down * normalForce;			// 중력 = 수직항력(-y)
				const Vec3 gravityAcc = gravityForce * mMass;				// 중력 가속도 = 중력 * 질량
				mVelocity += gravityAcc * DeltaTime();						// 속도 = 속도 + (중력가속도 * DeltaTime)
			}
		}
	}

	if (mVelocity.Length() > FLT_EPSILON) {
		// mVelocity 속도로 DeltaTime만큼 이동한다.
		mObject->Translate(mVelocity * DeltaTime());
	}
}


void Rigidbody::OnCollisionStay(Object& other)
{
	// 충돌처리 코드 필요...
}



void Rigidbody::Stop()
{
	SetVelocity(Vector3::Zero());
}

void Rigidbody::AddForce(const Vec3& force, ForceMode forceMode)
{
	float t{ 1 };
	if (forceMode == ForceMode::Accleration) {	// 가속도인 경우 DelteTime을 적용하도록 한다.
		t = DeltaTime();
	}

	const Vec3 acc = ((force * mAcc) / mMass) * t;	// 가속도 = ((force * mAcc) / 질량) * t
	mVelocity += acc;								// 현재 속도에 가속도를 더한다.
	if (mVelocity.Length()> mMaxSpeed) {	// 최대 속도를 넘지 않도록 한다.
		SetVelocity(mMaxSpeed);
	}
}

void Rigidbody::AddForce(const Vec3& dir, float speed, ForceMode forceMode)
{
	AddForce(dir * speed, forceMode);		// force = dir * speed
}