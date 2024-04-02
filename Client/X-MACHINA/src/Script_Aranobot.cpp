#include "stdafx.h"
#include "Script_Aranobot.h"

#include "InputMgr.h"

#include "Script_GroundObject.h"

#include "X-Engine.h"
#include "Timer.h"
#include "Object.h"
#include "AnimatorController.h"



void Script_Aranobot::Update()
{
	base::Update();

	if (KEY_PRESSED(VK_UP)) {
		mController->SetValue("Jump", true);
	}
	else {
		mController->SetValue("Jump", false);
	}

	if (KEY_PRESSED(VK_DOWN)) {
		mController->SetValue("Landing", true);
	}
	else {
		mController->SetValue("Landing", false);
	}

	if (KEY_PRESSED(VK_LEFT)) {
		mController->SetValue("Dodge", true);
	}
	else {
		mController->SetValue("Dodge", false);
	}
}