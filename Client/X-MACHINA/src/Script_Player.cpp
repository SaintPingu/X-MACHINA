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

void Script_Player::Start()
{
	base::Start();

	mHpBarUI = std::make_shared<SliderBarUI>("BackgroundHpBar", "EaseBar", "FillHpBar", Vec2{ 0.f, -425.f }, Vec2{ 1000.f, 15.f }, GetMaxHp());
}

void Script_Player::Update()
{
	base::Update();

	mHpBarUI->Update(GetCrntHp());
}

void Script_Player::Rotate(float pitch, float yaw, float roll)
{
	mObject->Rotate(pitch, yaw, roll);
}

bool Script_Player::Hit(float damage, Object* instigator)
{
	mHpBarUI->Decrease(GetCrntHp());

	bool res = base::Hit(damage, instigator);

	return res;
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