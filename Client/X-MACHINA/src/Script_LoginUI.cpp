#include "stdafx.h"
#include "Script_LoginUI.h"

#include "Script_Cursor.h"
#include "Script_LoginManager.h"

#include "TextMgr.h"
#include "PopupUI.h"

#include "Component/UI.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"


void Script_LoginUI::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_Cursor>();

	constexpr int kLoginSceneCnt = 6;
	int idx = rand() % kLoginSceneCnt;
	const auto& background = Canvas::I->CreateUI<UI>(0, "LoginBackground" + std::to_string(idx), Vec2::Zero, Vec2(Canvas::I->GetWidth(), Canvas::I->GetHeight()));
	Canvas::I->CreateUI<UI>(1, "LoginBackground_Default", Vec2(0, 100));

	{
		mLoginButton = Canvas::I->CreateUI<Button>(1, "LoginButton", Vec2(0, -300), Vec2(350, 49));
		mLoginButton->SetClickCallback(std::bind(&Script_LoginUI::SendLoginPacket, this));
		mLoginButton->SetHighlightTexture("LoginButton_H");
	}
	{
		mInput_ID = Canvas::I->CreateUI<InputField>(1, "InputField", Vec2(0, -50), Vec2(300, 30));
		mInput_ID->GetTextBox()->SetColor(TextFontColor::Type::White);
		mInput_ID->SetMaxLength(18);
		mInput_ID->SetOnlyEnglish();
	}
	{
		mInput_PW = Canvas::I->CreateUI<InputField>(1, "InputField", Vec2(0, -140), Vec2(300, 30));
		mInput_PW->GetTextBox()->SetColor(TextFontColor::Type::White);
		mInput_PW->SetMaxLength(18);
		mInput_PW->SetOnlyEnglish();
		mInput_PW->SetSecure();
	}
	
	{
		mLoginFailPopup = std::make_shared<PopupUI>("QuitPopup");
		const auto& okButton = Canvas::I->CreateUI<Button>(7, "YesButton", Vec2(-70, -70));
		okButton->SetHighlightTexture("YesHButton");
		okButton->SetClickCallback(std::bind(&Script_LoginUI::CloseLoginFailPopupCallback, this));
		mLoginFailPopup->AddUI(okButton);
		mLoginFailPopup->SetActive(false);
	}

	{
		mInput_ID->SetText(L"Player1");
		mInput_PW->SetText(L"Test1234");
	}
}

void Script_LoginUI::FailLogin()
{
	mLoginFailPopup->SetActive(true);
}

void Script_LoginUI::SendLoginPacket()
{
	// ID, Password ют╥б
	const std::string& ID = mInput_ID->GetText();
	const std::string& Password = mInput_PW->GetText();

	/* SEND LOGIN PACKET */
	auto CPktBuf = FBS_FACTORY->CPkt_LogIn(ID, Password);
	CLIENT_NETWORK->Send(CPktBuf);
}

void Script_LoginUI::CloseLoginFailPopupCallback()
{
	mLoginFailPopup->SetActive(false);
}
