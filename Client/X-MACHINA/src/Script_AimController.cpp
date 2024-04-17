#include "stdafx.h"
#include "Script_AimController.h"

#include "Component/UI.h"

#include "InputMgr.h"
#include "GameFramework.h"

void Script_AimController::Awake()
{
	base::Awake();

	mUI = Canvas::I->CreateUI("Aim", Vec2(0, 0), 30, 30);

	RESOLUTION resolution = GameFramework::I->GetWindowResolution();
	mMaxXPos = resolution.Width - 10.f;
	mMaxYPos = resolution.Height - 30.f;
}

void Script_AimController::Update()
{
	Vec2 mouseDelta = InputMgr::I->GetMouseDelta() * mouseSensitivity;

	mMousePos += mouseDelta;
	mMousePos.x = std::clamp(mMousePos.x, -mMaxXPos, mMaxXPos);
	mMousePos.y = std::clamp(mMousePos.y, -mMaxYPos, mMaxYPos);

	mUI->SetPosition(mMousePos);
}