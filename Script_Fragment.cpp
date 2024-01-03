#include "stdafx.h"
#include "Script_Fragment.h"

#include "Timer.h"
#include "Rigidbody.h"

void Script_Fragment::Start()
{
	mRigid = mObject->AddComponent<Rigidbody>();
	mRigid->SetGravity(true);
	mRigid->SetGravityScale(3.f);
	mRigid->SetFriction(.2f);
	mRigid->SetMass(5.f);
	mRigid->SetDrag(1.f);
}

void Script_Fragment::Update()
{
	mObject->Rotate(mRotationAxis, mRotationSpeed * DeltaTime());
}

void Script_Fragment::Active(const Vec3& pos)
{
	mObject->SetPosition(pos);
	mRigid->Stop();

	// 위 방향 + 설정된 [mMovingDir] 방향으로 즉시 힘을 가한다.
	mRigid->AddForce(Vector3::Up(), mMovingSpeed, ForceMode::Impulse);
	mRigid->AddForce(mMovingDir, mMovingSpeed, ForceMode::Impulse);
}