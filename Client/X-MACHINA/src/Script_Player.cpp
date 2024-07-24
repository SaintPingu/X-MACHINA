#include "stdafx.h"
#include "Script_Player.h"

#include "Script_MainCamera.h"

#include "Component/Camera.h"
#include "Component/Rigidbody.h"
#include "Component/UI.h"

#include "Object.h"

#include "SliderBarUI.h"
#include "ChatBoxUI.h"

namespace {
	BoundingBox border = { Vec3(256, 100, 256), Vec3(240, 1100, 240) };
}



bool Script_Player::IsActiveChatBox() const
{
	return mChatBoxUI->IsActive();
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

	mHpBarUI = std::make_shared<SliderBarUI>("BackgroundHpBar", "EaseBar", "FillHpBar", Vec2{ 0.f, -850.f }, Vec2{ 1000.f, 15.f }, GetMaxHp());
	mChatBoxUI = std::make_shared<ChatBoxUI>(Vec2{ -1500.f, -400.f }, Vec2{300.f, 150.f}, "Name");
}

#include "Timer.h"
void Script_Player::Update()
{
	base::Update();

	Vec3 pos = mObject->GetPosition();

	if (!border.Contains(_VECTOR(pos))) {
		mObject->ReturnToPrevTransform();
	}

	mHpBarUI->Update(GetCrntHp());
	mChatBoxUI->Update();
}

bool Script_Player::ProcessInput()
{
	if (mChatBoxUI->IsActive()) {
		return false;
	}

	return true;
}

bool Script_Player::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	if (!mChatBoxUI) {
		return true;
	}

	switch (messageID) {
	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			mChatBoxUI->ToggleChatBox();
			return false;
		}
	}

	if (mChatBoxUI->IsActive()) {
		mChatBoxUI->ProcessKeyboardMsg(messageID, wParam, lParam);
		return false;
	}

	return true;
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

void Script_Player::AddScore(int score)
{
	mScore += score;
}