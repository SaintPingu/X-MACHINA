#include "stdafx.h"
#include "Script_Player.h"

#include "Script_MainCamera.h"

#include "Component/Camera.h"
#include "Component/Rigidbody.h"
#include "Component/UI.h"

#include "Object.h"

namespace {
	BoundingBox border = { Vec3(256, 100, 256), Vec3(240, 1100, 240) };
}



void Script_Player::SetSpawn(const Vec3& pos)
{
	mObject->SetPosition(pos);
	XMStoreFloat4x4(&mSpawnTransform, _MATRIX(mObject->GetWorldTransform()));
}

void Script_Player::Awake()
{
	base::Awake();

	mCamera = MainCamera::I->GetComponent<Script_MainCamera>().get();
}

void Script_Player::Start()
{
	base::Start();

	mTarget = mObject->GetObj<GameObject>();
}

void Script_Player::Update()
{
	base::Update();

	Vec3 pos = mObject->GetPosition();

	if (!border.Contains(_VECTOR(pos))) {
		mObject->ReturnToPrevTransform();
	}
}

void Script_Player::Rotate(float pitch, float yaw, float roll)
{
	mObject->Rotate(pitch, yaw, roll);
}

void Script_Player::Dead()
{
	Respawn();
}

void Script_Player::Respawn()
{
	mObject->SetWorldTransform(mSpawnTransform);
}

void Script_Player::AddScore(int score)
{
	mScore += score;
	Canvas::I->SetScore(mScore);
}