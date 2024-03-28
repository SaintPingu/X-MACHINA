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

	// �������� �����Ѵ�.
	if (mFriction > FLT_EPSILON) {
		float normalForce{ mMass };		// �����׷�
		if (mUseGravity) {
			normalForce *= gravity;		// ���� �׷¿� �߷��� �����Ѵ�.
		}
		
		const float friction = mFriction * normalForce;		// ���� ��� = ���� ��� * �����׷�

		const float mag = mVelocity.Length();		// ���� �ӵ� ũ��
		if (mag > FLT_EPSILON) {							// ���� �ӵ��� �ִٸ�
			const Vec3 frictionDir = -mVelocity / mag;	// ������ ����(���� �ӵ��� �ݴ����)

			const Vec3 frictionForce = frictionDir * friction;			// ������ = ������ ���� * �������
			const Vec3 dragForce = mVelocity * -mDrag;					// ���׷� = �ӵ� * ���� ��� * -1
			const Vec3 dragAcc = dragForce / mMass;							// ���� ���ӵ� = ���׷� * ����

			const Vec3 frictionAcc = (frictionForce / mMass) + dragAcc;	// ���� ���ӵ� = (������/����) + ���� ���ӵ�

			const Vec3 resultVec = mVelocity + (frictionAcc * DeltaTime());	// ��� = ���� �ӵ� + (���� ���ӵ� * DeltaTime)

			// �� ������ ������ �ٲ�� 0���� �����Ѵ�.
			mVelocity.x = (mVelocity.x * resultVec.x < 0) ? 0.f : resultVec.x;
			mVelocity.y = (mVelocity.y * resultVec.y < 0) ? 0.f : resultVec.y;
			mVelocity.z = (mVelocity.z * resultVec.z < 0) ? 0.f : resultVec.z;

			// �߷��� Ȱ��ȭ�� ��� �߷� ���ӵ��� ����� �ӵ��� �����Ѵ�.
			if (mUseGravity) {
				const Vec3 gravityForce = Vec3::Down * normalForce;			// �߷� = �����׷�(-y)
				const Vec3 gravityAcc = gravityForce * mMass;				// �߷� ���ӵ� = �߷� * ����
				mVelocity += gravityAcc * DeltaTime();						// �ӵ� = �ӵ� + (�߷°��ӵ� * DeltaTime)
			}
		}
	}

	if (mVelocity.Length() > FLT_EPSILON) {
		// mVelocity �ӵ��� DeltaTime��ŭ �̵��Ѵ�.
		mObject->Translate(mVelocity * DeltaTime());
	}
}


void Rigidbody::OnCollisionStay(Object& other)
{
	// �浹ó�� �ڵ� �ʿ�...
}



void Rigidbody::Stop()
{
	SetVelocity(Vector3::Zero());
}

void Rigidbody::AddForce(const Vec3& force, ForceMode forceMode)
{
	float t{ 1 };
	if (forceMode == ForceMode::Accleration) {	// ���ӵ��� ��� DelteTime�� �����ϵ��� �Ѵ�.
		t = DeltaTime();
	}

	const Vec3 acc = ((force * mAcc) / mMass) * t;	// ���ӵ� = ((force * mAcc) / ����) * t
	mVelocity += acc;								// ���� �ӵ��� ���ӵ��� ���Ѵ�.
	if (mVelocity.Length()> mMaxSpeed) {	// �ִ� �ӵ��� ���� �ʵ��� �Ѵ�.
		SetVelocity(mMaxSpeed);
	}
}

void Rigidbody::AddForce(const Vec3& dir, float speed, ForceMode forceMode)
{
	AddForce(dir * speed, forceMode);		// force = dir * speed
}