#include "stdafx.h"
#include "Script_Player.h"
#include "Scene.h"
#include "Shader.h"
#include "InputMgr.h"
#include "Timer.h"
#include "Rigidbody.h"






void Script_AirplanePlayer::Start()
{
	Script_ShootingPlayer::Start();

	mGunFirePos = mPlayer->FindFrame("gunfire_pos");

	SetFireDelay(0.1f);
	mPlayerType = PlayerType::Airplane;
	mRotationSpeed = 90.0f;

	SetSpawn(Vec3(50.0f, 100.0f, 50.0f));
	SetHP(150.0f);
	SetDamage(10.0f);
	mObject->GetComponent<Rigidbody>()->SetMass(100.f);
	mObject->GetComponent<Rigidbody>()->SetFriction(30.f);

	base::SetAcc(5000.0f);
	base::SetMaxSpeed(100.0f);
	mBulletSpeed = 300.f;
}


void Script_AirplanePlayer::Update()
{
	base::Update();

	base::ProcessInput();
	ProcessInput();

	Vec3 pos = mObject->GetPosition();
	float terrainHeight = crntScene->GetTerrainHeight(pos.x, pos.z);

	if (pos.y < terrainHeight) {
		Explode();
	}
}

void Script_AirplanePlayer::ProcessInput()
{
	DWORD dwDirection = 0;
	DWORD rotationDir = 0;
	if (KEY_PRESSED('W'))			dwDirection |= DIR_FORWARD;
	if (KEY_PRESSED('S'))			dwDirection |= DIR_BACKWARD;
	if (KEY_PRESSED(VK_SPACE))		dwDirection |= DIR_UP;
	if (KEY_PRESSED(VK_SHIFT))		dwDirection |= DIR_DOWN;
	if (dwDirection) {
		base::Move(dwDirection);
	}

	if (KEY_PRESSED('A')) rotationDir |= DIR_LEFT;
	if (KEY_PRESSED('D')) rotationDir |= DIR_RIGHT;
	if (rotationDir) {
		Rotate(rotationDir, DeltaTime());
	}

	if (KEY_PRESSED(VK_LBUTTON)) {
		Vec2 mouseDelta = InputMgr::Inst()->GetMouseDelta();
		mPlayer->Rotate(mouseDelta.y, mouseDelta.x);
	}
}



void Script_AirplanePlayer::Rotate(DWORD rotationDir, float angle)
{
	angle *= mRotationSpeed;
	if (rotationDir) {
		float zRotation = 0.0f;
		if (rotationDir & DIR_LEFT) {
			zRotation += angle;
		}
		if (rotationDir & DIR_RIGHT) {
			zRotation -= angle;
		}

		if (zRotation != 0.0f) {
			mObject->Rotate(0.0f, 0.0f, zRotation);
		}
	}
}


void Script_AirplanePlayer::FireBullet()
{
	if (mGunFirePos) {
		Vec3 position = mGunFirePos->GetPosition();
		Vec3 dir = mGunFirePos->GetLook();
		Vec3 up = mGunFirePos->GetUp();

		mBulletShader->FireBullet(position, dir, up, mBulletSpeed);
	}
}


void Script_AirplanePlayer::OnCollisionStay(Object& other)
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