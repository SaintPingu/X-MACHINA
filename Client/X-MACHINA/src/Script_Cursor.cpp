#include "stdafx.h"
#include "Script_Cursor.h"

#include "Script_AimController.h"

#include "Component/UI.h"
#include "InputMgr.h"

void Script_Cursor::Awake()
{
	base::Awake();

	mCursorNormal = Canvas::I->CreateUI<UI>(9, "Cursor_Normal", Vec2::Zero, Vec2(60, 60));
	mCursorClick = Canvas::I->CreateUI<UI>(9, "Cursor_Click", Vec2::Zero, Vec2(60, 60));
	mCursorClick->SetActive(false);

	mAimController = mObject->AddComponent<Script_AimController>();
	mAimController->SetIconUI(mCursorNormal);
}

void Script_Cursor::Update()
{
	base::Update();

	if (KEY_TAP(VK_LBUTTON)) {
		Canvas::I->CheckClick();

		mCursorNormal->SetActive(false);
		mCursorClick->SetActive(true);
		mAimController->SetIconUI(mCursorClick);
	}
	else if (KEY_AWAY(VK_LBUTTON)) {
		mCursorNormal->SetActive(true);
		mCursorClick->SetActive(false);
		mAimController->SetIconUI(mCursorNormal);
	}
}