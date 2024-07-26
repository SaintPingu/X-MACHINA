#include "stdafx.h"
#include "Script_LobbyUI.h"

#include "Script_AimController.h"
#include "Script_LobbyManager.h"

#include "PopupUI.h"

#include "Component/UI.h"
#include "InputMgr.h"
#include "TextMgr.h"
#include "SoundMgr.h"

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
		constexpr float y    = -165;
		constexpr float yGap = -65;
				
		const auto& playButton    = Canvas::I->CreateUI<Button>(1, "PlayButton", Vec2(x, y));
		playButton->SetHighlightTexture("PlayHButton");
		playButton->SetClickCallback(std::bind(&Script_LobbyUI::PlayButton, this));
		const auto& customButton  = Canvas::I->CreateUI<Button>(1, "CustomButton", Vec2(x, y + (yGap * 1)));
		customButton->SetHighlightTexture("CustomHButton");
		const auto& settingButton = Canvas::I->CreateUI<Button>(1, "SettingButton", Vec2(x, y + (yGap * 2)));
		settingButton->SetHighlightTexture("SettingHButton");
		const auto& quitButton    = Canvas::I->CreateUI<Button>(1, "QuitButton", Vec2(x, y + (yGap * 3)));
		quitButton->SetHighlightTexture("QuitHButton");
		quitButton->SetClickCallback(std::bind(&Script_LobbyUI::ShowQuitPopup, this));

		playButton->SetClickSound("ButtonClick");
		customButton->SetClickSound("ButtonClick");
		settingButton->SetClickSound("ButtonClick");
		quitButton->SetClickSound("ButtonClick");

		playButton->SetHoverSound("ButtonHighlight");
		customButton->SetHoverSound("ButtonHighlight");
		settingButton->SetHoverSound("ButtonHighlight");
		quitButton->SetHoverSound("ButtonHighlight");
	}

	// Popup
	{
		mPopupQuit = std::make_shared<PopupUI>("QuitPopup");
		const auto& yesButton = Canvas::I->CreateUI<Button>(7, "YesButton", Vec2(-70, -70));
		yesButton->SetHighlightTexture("YesHButton");
		yesButton->SetClickCallback(std::bind(&Script_LobbyUI::QuitGame, this));
		const auto& noButton = Canvas::I->CreateUI<Button>(7, "NoButton", Vec2(100, -70));
		noButton->SetHighlightTexture("NoHButton");
		noButton->SetClickCallback(std::bind(&Script_LobbyUI::HideQuitPopup, this));
		mPopupQuit->AddUI(yesButton);
		mPopupQuit->AddUI(noButton);
		mPopupQuit->SetActive(false);
	}


	mCursorNormal = Canvas::I->CreateUI<UI>(9, "Cursor_Normal", Vec2::Zero, Vec2(60, 60));
	mCursorClick = Canvas::I->CreateUI<UI>(9, "Cursor_Click", Vec2::Zero, Vec2(60, 60));
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

void Script_LobbyUI::PlayButton() const
{
	Script_LobbyManager::ChangeToBattleScene();
}

void Script_LobbyUI::QuitGame() const
{
	::PostQuitMessage(0);
}

void Script_LobbyUI::ShowQuitPopup() const
{
	mPopupQuit->SetActive(true);
}

void Script_LobbyUI::HideQuitPopup() const
{
	mPopupQuit->SetActive(false);
}