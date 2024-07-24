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

	// Panels
	{
		const auto& top = Canvas::I->CreateUI<UI>(0, "Logo", Vec2(-800, 450));
		const auto& bottm = Canvas::I->CreateUI<UI>(0, "BottomPanel", Vec2(0, -470));
	}

	// Buttons
	{
		constexpr float x    = -720;
		constexpr float y    = -150;
		constexpr float yGap = -55;
				
		const auto& playButton    = Canvas::I->CreateUI<Button>(1, "PlayButton", Vec2(x, y));
		playButton->SetHighlightTexture("PlayHButton");
		const auto& customButton  = Canvas::I->CreateUI<Button>(1, "CustomButton", Vec2(x, y + (yGap * 1)));
		customButton->SetHighlightTexture("CustomHButton");
		const auto& settingButton = Canvas::I->CreateUI<Button>(1, "SettingButton", Vec2(x, y + (yGap * 2)));
		settingButton->SetHighlightTexture("SettingHButton");
		const auto& quitButton    = Canvas::I->CreateUI<Button>(1, "QuitButton", Vec2(x, y + (yGap * 3)));
		quitButton->SetHighlightTexture("QuitHButton");
	}

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
		Canvas::I->CheckClick();

		mCursorNormal->SetActive(false);
		mCursorClick->SetActive(true);
		mAimController->SetUI(mCursorClick);
		;
	}
	else if (KEY_AWAY(VK_LBUTTON)) {
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