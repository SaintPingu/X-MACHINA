#include "stdafx.h"
#include "Script_LobbyUI.h"

#include "Script_AimController.h"
#include "Script_LobbyManager.h"

#include "Component/UI.h"
#include "InputMgr.h"
#include "TextMgr.h"

void Script_LobbyUI::Start()
{
	base::Start();

	const auto& start = Canvas::I->CreateUI<Button>(0, "StartButton", Vec2(-1600, -900), Vec2(200, 80));
	start->SetPressedTexture("TopPanel");
	//start->AddClickCallback(std::bind(&Script_LobbyUI::ChangeToBattleScene, this));

	const auto& topPanel = Canvas::I->CreateUI<UI>(0, "TopPanel", Vec2(0, 1080 - 150), Vec2(1920, 150));

	mCursorNormal = Canvas::I->CreateUI<UI>(4, "Cursor_Normal", Vec2::Zero, Vec2(60, 60));
	mCursorClick = Canvas::I->CreateUI<UI>(4, "Cursor_Click", Vec2::Zero, Vec2(60, 60));
	mCursorClick->SetActive(false);

	mAimController = mObject->AddComponent<Script_AimController>();
	mAimController->SetUI(mCursorNormal);
}

void Script_LobbyUI::Update()
{
	base::Update();

	if (KEY_TAP(VK_LBUTTON)) {
		Vec2 pos = mAimController->GetAimNDCPos();
		Canvas::I->CheckClick(pos);

		mCursorNormal->SetActive(false);
		mCursorClick->SetActive(true);
		mAimController->SetUI(mCursorClick);
		;
	}
	else if (KEY_AWAY(VK_LBUTTON)) {
		Vec2 pos = mAimController->GetAimNDCPos();

		mCursorNormal->SetActive(true);
		mCursorClick->SetActive(false);
		mAimController->SetUI(mCursorNormal);
	}
}

void Script_LobbyUI::OnDestroy()
{
	base::OnDestroy();

	mObject->RemoveComponent<Script_AimController>();
}

void Script_LobbyUI::ChangeToBattleScene() const
{
	//Script_LobbyManager::ChangeToBattleScene();
}