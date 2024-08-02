#include "stdafx.h"
#include "Script_Player.h"

#include "Script_MainCamera.h"

#include "Component/Camera.h"
#include "Component/Rigidbody.h"
#include "Component/UI.h"

#include "Object.h"

#include "SliderBarUI.h"
#include "ChatBoxUI.h"

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
	Resurrect();
	mObject->SetWorldTransform(mSpawnTransform);
}