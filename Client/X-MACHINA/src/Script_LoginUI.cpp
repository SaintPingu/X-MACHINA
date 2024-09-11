#include "stdafx.h"
#include "Script_LoginUI.h"

#include "Script_Cursor.h"
#include "Script_LoginManager.h"

#include "Component/UI.h"


void Script_LoginUI::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_Cursor>();

	constexpr int kLoginSceneCnt = 6;
	int idx = rand() % kLoginSceneCnt;
	Canvas::I->CreateUI<UI>(0, "LoginBackground" + std::to_string(idx), Vec2::Zero, Vec2(Canvas::I->GetWidth(), Canvas::I->GetHeight()));
}
