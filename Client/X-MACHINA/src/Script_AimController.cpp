#include "stdafx.h"
#include "Script_AimController.h"

#include "Component/UI.h"

#include "InputMgr.h"
#include "GameFramework.h"

void Script_AimController::Awake()
{
	base::Awake();

	RESOLUTION resolution = GameFramework::I->GetWindowResolution();
	mMaxPos.x = resolution.Width - 10.f;
	mMaxPos.y = resolution.Height - 30.f;
}

void Script_AimController::Update()
{
	base::Update();

	const Vec2 mouseDelta = InputMgr::I->GetMouseDelta() * mouseSensitivity;

	mMousePos += mouseDelta;
	mMousePos.x = std::clamp(mMousePos.x, -mMaxPos.x, mMaxPos.x);
	mMousePos.y = std::clamp(mMousePos.y, -mMaxPos.y, mMaxPos.y);

	if (mUI) {
		mUI->SetPosition(mMousePos);
	}
}

Vec2 Script_AimController::GetScreenAimPos() const
{
	float windowWidth = static_cast<float>(GameFramework::I->GetWindowResolution().Width);
	float windowHeight = static_cast<float>(GameFramework::I->GetWindowResolution().Height);

	Vec2 screenPos = mMousePos / 2.f;
	screenPos.y = 1.f - screenPos.y;
	screenPos += Vec2{ windowWidth, windowHeight } / 2.f;

	return screenPos;
}

UITexture* Script_AimController::GetUITexture() 
{
	assert(mUI);
	return mUI->GetUITexture();
}

void Script_AimController::ChangeAimUITexture(UITexture* newUITexture)
{
	if (mUI) {
		mUI->ChangeUITexture(newUITexture->shared_from_this());
	}
}
