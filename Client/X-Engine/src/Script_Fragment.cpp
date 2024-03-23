#include "stdafx.h"
#include "Script_Fragment.h"

#include "Timer.h"
#include "Rigidbody.h"

void Script_Fragment::Awake()
{
	const auto& rigid = mObject->GetComponent<Rigidbody>();
	rigid->SetGravity(true);
	rigid->SetGravityScale(3.f);
	rigid->SetFriction(.2f);
	rigid->SetMass(5.f);
	rigid->SetDrag(1.f);
}

void Script_Fragment::Update()
{
	mObject->Rotate(mRotationAxis, mRotationSpeed * DeltaTime());
}

void Script_Fragment::Active(const Vec3& pos)
{
	mObject->SetPosition(pos);

	const auto& rigid = mObject->GetComponent<Rigidbody>();

	rigid->Stop();
	// �� ���� + ������ [mMovingDir] �������� ��� ���� ���Ѵ�.
	rigid->AddForce(Vector3::Up(), mMovingSpeed, ForceMode::Impulse);
	rigid->AddForce(mMovingDir, mMovingSpeed, ForceMode::Impulse);

	mObject->OnEnable();
}