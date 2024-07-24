#include "stdafx.h"
#include "Script_LobbyUI.h"

#include "Script_AimController.h"
#include "Script_LobbyManager.h"

#include "Component/UI.h"
#include "InputMgr.h"
#include "TextMgr.h"

void Script_LobbyUI::Start()
{
	const auto& start = Canvas::I->CreateUI(0, "Title", { -1100, 0 }, 200, 100);
	start->AddClickCallback(std::bind(&Script_LobbyUI::ChangeToBattleScene, this));

	//const auto& menu = Canvas::I->CreateUI(0, "Menu", { 0, 1080 - 150 }, 1920, 150);
	const auto& layout = Canvas::I->CreateUI(0, "Layout", { 0, 0 }, 1920, 1080);

	mCursorNormal = Canvas::I->CreateUI(4, "Cursor_Normal", Vec2::Zero, 60, 60);
	mCursorClick = Canvas::I->CreateUI(4, "Cursor_Click", Vec2::Zero, 60, 60);
	mCursorClick->SetActive(false);

	{
		TextOption textOption;
		textOption.FontSize = 72.f;
		textOption.FontColor = TextFontColor::Type::WhiteSmoke;
		textOption.FontWeight = TextFontWeight::EXTRA_BOLD;

		TextMgr::I->CreateText("PLAY", Vec2(0, -480), textOption);
	}

	mAimController = mObject->AddComponent<Script_AimController>();
	mAimController->SetUI(mCursorNormal);
}

void Script_LobbyUI::Update()
{
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
	mObject->RemoveComponent<Script_AimController>();
}

void Script_LobbyUI::ChangeToBattleScene() const
{
	Script_LobbyManager::ChangeToBattleScene();
}