#include "stdafx.h"
#include "Script_AimController.h"

#include "Component/UI.h"

#include "InputMgr.h"
#include "GameFramework.h"

void Script_AimController::Awake()
{
	mUI = canvas->CreateUI("Aim", Vec2(0, 0), 30, 30);
	mUI2 = canvas->CreateUI("Aim", Vec2(0, 0), 20, 20);

	RESOLUTION resolution = framework->GetWindowResolution();
	mMaxXPos = resolution.Width - 10.f;
	mMaxYPos = resolution.Height - 30.f;
}

void Script_AimController::Update()
{
	Vec2 mouseDelta = input->GetMouseDelta() * mouseSensitivity;

	mMousePos += mouseDelta;
	mMousePos.x = std::clamp(mMousePos.x, -mMaxXPos, mMaxXPos);
	mMousePos.y = std::clamp(mMousePos.y, -mMaxYPos, mMaxYPos);

	mUI->SetPosition(mMousePos);
}