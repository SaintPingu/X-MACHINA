#include "stdafx.h"
#include "Script_Player.h"

#include "Scene.h"
#include "Object.h"
#include "InputMgr.h"
#include "Rigidbody.h"

#include "Animator.h"






void Script_GroundPlayer::Start()
{
	base::Start();

	mPlayerType = PlayerType::Human;
	mRotationSpeed = 60.f;

	SetSpawn(Vec3(50.f, 100.f, 50.f));
	SetHP(150.f);

	mRigid->SetMass(100.f);
	mRigid->SetFriction(30.f);
	mRigid->SetAcc(5000.f);
	mRigid->SetMaxSpeed(5.5f);
}


void Script_GroundPlayer::Update()
{
	base::Update();

	base::ProcessInput();
	ProcessInput();

	Vec3 pos = mObject->GetPosition();
	float terrainHeight = scene->GetTerrainHeight(pos.x, pos.z);
	mObject->SetPositionY(terrainHeight);
}

void Script_GroundPlayer::ProcessInput()
{
	DWORD dwDirection = 0;
	DWORD rotationDir = 0;
	if (KEY_PRESSED('W'))			dwDirection |= Dir::Front;
	if (KEY_PRESSED('S'))			dwDirection |= Dir::Back;
	if (KEY_PRESSED('A'))			dwDirection |= Dir::Left;
	if (KEY_PRESSED('D'))			dwDirection |= Dir::Right;
	if (dwDirection) {
		base::Move(dwDirection);
	}

	if (KEY_PRESSED(VK_LCONTROL)) {
		mObject->GetObj<GameObject>()->GetAnimator()->SetBool("Sit", true);
	}
	else {
		mObject->GetObj<GameObject>()->GetAnimator()->SetBool("Sit", false);
	}

	if (Vector3::Length(mRigid->GetVelocity()) > 0.1f) {
		mObject->GetObj<GameObject>()->GetAnimator()->SetBool("Walk", true);
	}
	else {
		mObject->GetObj<GameObject>()->GetAnimator()->SetBool("Walk", false);
	}

	if (KEY_PRESSED(VK_LBUTTON)) {
		Vec2 mouseDelta = InputMgr::Inst()->GetMouseDelta();
		mPlayer->Rotate(0.f, mouseDelta.x);
	}
}



void Script_GroundPlayer::Rotate(DWORD rotationDir, float angle)
{
	angle *= mRotationSpeed;
	if (rotationDir) {
		float zRotation = 0.f;
		if (rotationDir & Dir::Left) {
			zRotation += angle;
		}
		if (rotationDir & Dir::Right) {
			zRotation -= angle;
		}

		if (zRotation != 0.f) {
			mObject->Rotate(0.f, 0.f, zRotation);
		}
	}
}


void Script_GroundPlayer::FireBullet()
{
	// TODO : FireBullet
}


void Script_GroundPlayer::OnCollisionStay(Object& other)
{
	switch (other.GetTag())
	{
	case ObjectTag::Building:
		Explode();
		break;
	default:
		break;
	}
}