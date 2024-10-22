#include "stdafx.h"
#include "Script_LobbyUI.h"

#include "Script_Cursor.h"
#include "Script_LobbyManager.h"

#include "PopupUI.h"

#include "Component/UI.h"
#include "InputMgr.h"
#include "TextMgr.h"
#include "SoundMgr.h"

#include "ClientNetwork/Contents/ClientNetworkManager.h"
#include "ClientNetwork/Contents/FBsPacketFactory.h"

void Script_LobbyUI::Awake()
{
	base::Awake();

	// Panels
	{
		const auto& top = Canvas::I->CreateUI<UI>(0, "LobbyLogo", Vec2(-500, 250));
	}

	// Buttons
	{
		constexpr float x    = -500;
		constexpr float y    = 100;
		constexpr float yGap = -125;
				
		const auto& playButton    = Canvas::I->CreateUI<Button>(1, "PlayButton", Vec2(x, y));
		playButton->SetHighlightTexture("PlayHButton");
		playButton->SetClickCallback(std::bind(&Script_LobbyUI::PlayButton, this));
		const auto& customButton  = Canvas::I->CreateUI<Button>(1, "CustomButton", Vec2(x, y + (yGap * 1)));
		customButton->SetHighlightTexture("CustomHButton");
		customButton->SetClickCallback(std::bind(&Script_LobbyUI::CustomButton, this));
		const auto& settingButton = Canvas::I->CreateUI<Button>(1, "SettingButton", Vec2(x, y + (yGap * 2)));
		settingButton->SetHighlightTexture("SettingHButton");
		const auto& quitButton    = Canvas::I->CreateUI<Button>(1, "QuitButton", Vec2(x, y + (yGap * 3)));
		quitButton->SetHighlightTexture("QuitHButton");
		quitButton->SetClickCallback(std::bind(&Script_LobbyUI::ShowQuitPopup, this));

		mCustomLeftArrow = Canvas::I->CreateUI<Button>(1, "ArrowButton", Vec2(100, -170), Vec2(50, 50));
		mCustomLeftArrow->SetClickCallback(std::bind(&Script_LobbyUI::CustomLeftArrowButton, this));
		mCustomLeftArrow->SetHighlightTexture("ArrowHButton");
		mCustomLeftArrow->Rotate(Vector3::Forward, 180);
		mCustomLeftArrow->SetActive(false);
		mCustomRightArrow = Canvas::I->CreateUI<Button>(1, "ArrowButton", Vec2(520, -170), Vec2(50, 50));
		mCustomRightArrow->SetClickCallback(std::bind(&Script_LobbyUI::CustomRightArrowButton, this));
		mCustomRightArrow->SetHighlightTexture("ArrowHButton");
		mCustomRightArrow->SetActive(false);

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

		noButton->SetClickSound("BackButtonClick");
	}

	mObject->AddComponent<Script_Cursor>();
}

void Script_LobbyUI::PlayButton() const
{
	auto cpkt = FBS_FACTORY->CPkt_PlayGame();
	CLIENT_NETWORK->Send(cpkt);
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

void Script_LobbyUI::CustomButton()
{
	mIsInCustom = !mIsInCustom;

	if (mIsInCustom) {
		mCustomLeftArrow->SetActive(true);
		mCustomRightArrow->SetActive(true);
	}
	else {
		mCustomLeftArrow->SetActive(false);
		mCustomRightArrow->SetActive(false);
	}
}

void Script_LobbyUI::CustomLeftArrowButton()
{
	mLobbyManager->ChagneToPrevSkin();
}

void Script_LobbyUI::CustomRightArrowButton()
{
	mLobbyManager->ChagneToNextSkin();
}
