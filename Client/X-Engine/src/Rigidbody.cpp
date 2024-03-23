#include "stdafx.h"
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

		const float mag = Vector3::Length(mVelocity);		// ���� �ӵ� ũ��
		if (mag > FLT_EPSILON) {							// ���� �ӵ��� �ִٸ�
			const Vec3 frictionDir = Vector3::Divide(-mVelocity, mag);	// ������ ����(���� �ӵ��� �ݴ����)

			const Vec3 frictionForce = Vector3::Multiply(frictionDir, friction);			// ������ = ������ ���� * �������
			const Vec3 dragForce = Vector3::Multiply(mVelocity, -mDrag);					// ���׷� = �ӵ� * ���� ��� * -1
			const Vec3 dragAcc = Vector3::Divide(dragForce, mMass);							// ���� ���ӵ� = ���׷� * ����

			const Vec3 frictionAcc = Vector3::Add(Vector3::Divide(frictionForce, mMass), dragAcc);	// ���� ���ӵ� = (������/����) + ���� ���ӵ�

			const Vec3 resultVec = Vector3::Add(mVelocity, Vector3::Multiply(frictionAcc, DeltaTime()));	// ��� = ���� �ӵ� + (���� ���ӵ� * DeltaTime)

			// �� ������ ������ �ٲ�� 0���� �����Ѵ�.
			mVelocity.x = (mVelocity.x * resultVec.x < 0) ? 0.f : resultVec.x;
			mVelocity.y = (mVelocity.y * resultVec.y < 0) ? 0.f : resultVec.y;
			mVelocity.z = (mVelocity.z * resultVec.z < 0) ? 0.f : resultVec.z;

			// �߷��� Ȱ��ȭ�� ��� �߷� ���ӵ��� ����� �ӵ��� �����Ѵ�.
			if (mUseGravity) {
				const Vec3 gravityForce = Vector3::Multiply(Vector3::Down(), normalForce);			// �߷� = �����׷�(-y)
				const Vec3 gravityAcc = Vector3::Divide(gravityForce, mMass);						// �߷� ���ӵ� = �߷� * ����
				mVelocity = Vector3::Add(mVelocity, Vector3::Multiply(gravityAcc, DeltaTime()));	// �ӵ� = �ӵ� + (�߷°��ӵ� * DeltaTime)
			}
		}
	}

	// mVelocity �ӵ��� DeltaTime��ŭ �̵��Ѵ�.
	mObject->Translate(Vector3::Multiply(mVelocity, DeltaTime()));
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

	const Vec3 acc = Vector3::Multiply(Vector3::Divide(Vector3::Multiply(force, mAcc), mMass), t);	// ���ӵ� = ((force * mAcc) / ����) * t
	mVelocity = Vector3::Add(mVelocity, acc);		// ���� �ӵ��� ���ӵ��� ���Ѵ�.
	if (Vector3::Length(mVelocity) > mMaxSpeed) {	// �ִ� �ӵ��� ���� �ʵ��� �Ѵ�.
		SetVelocity(mMaxSpeed);
	}
}

void Rigidbody::AddForce(const Vec3& dir, float speed, ForceMode forceMode)
{
	AddForce(Vector3::Multiply(dir, speed), forceMode);		// force = dir * speed
}