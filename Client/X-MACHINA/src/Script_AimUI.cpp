#include "stdafx.h"
#include "Script_AimUI.h"

#include "Component/UI.h"

void Script_AimUI::Awake()
{
	mUI = canvas->CreateUI("Aim", Vec2(0, 0), 30, 30);
}
