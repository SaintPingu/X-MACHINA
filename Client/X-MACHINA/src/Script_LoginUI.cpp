#include "stdafx.h"
#include "Script_LoginUI.h"

#include "Script_Cursor.h"
#include "Script_LoginManager.h"

#include "TextMgr.h"
#include "PopupUI.h"
#include "Timer.h"

#include "Component/UI.h"

#include "ClientNetwork/Contents/FBsPacketFactory.h"
#include "ClientNetwork/Contents/ClientNetworkManager.h"


void Script_LoginUI::Awake()
{
	base::Awake();

	mObject->AddComponent<Script_Cursor>();

	{
		Canvas::I->CreateUI<UI>(0, "Black", Vec2::Zero, Vec2(Canvas::I->GetWidth(), Canvas::I->GetHeight()));
		mCurBackgroundIdx = 0;
		mBackground = Canvas::I->CreateUI<UI>(1, "LoginBackground" + GetNextBackgroundIdx(), Vec2::Zero, Vec2(Canvas::I->GetWidth(), Canvas::I->GetHeight()));
		Canvas::I->CreateUI<UI>(2, "LoginBackground_Default", Vec2(0, 100));
	}

	{
		mLoginButton = Canvas::I->CreateUI<Button>(2, "LoginButton", Vec2(-10, -300), Vec2(350, 49));
		mLoginButton->SetClickCallback(std::bind(&Script_LoginUI::SendLoginPacket, this));
		mLoginButton->SetHighlightTexture("LoginButton_H");
	}
	{
		mInput_ID = Canvas::I->CreateUI<InputField>(2, "InputField", Vec2(-10, -50), Vec2(276, 40));
		mInput_ID->GetTextBox()->SetColor(TextFontColor::Type::White);
		mInput_ID->SetMaxLength(10);
		mInput_ID->SetOnlyEnglish();
	}
	{
		mInput_PW = Canvas::I->CreateUI<InputField>(2, "InputField", Vec2(-10, -140), Vec2(276, 40));
		mInput_PW->GetTextBox()->SetColor(TextFontColor::Type::White);
		mInput_PW->SetMaxLength(12);
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
		mInput_PW->SetText(L"test1234");
	}

	mFadeSpeed = 0.25f;
}

void Script_LoginUI::Update()
{
	base::Update();

	if (mFade_t <= 0) {
		mCurChangeBackgroundDelay += DeltaTime();
		if (mCurChangeBackgroundDelay >= mkMaxChangeBackgroundDelay) {
			mCurChangeBackgroundDelay = 0.f;
			mFade_t = 1.f;
			mBackgroundAfter = Canvas::I->CreateUI<UI>(0, "LoginBackground" + GetNextBackgroundIdx(), Vec2::Zero, Vec2(Canvas::I->GetWidth(), Canvas::I->GetHeight()));
			mBackgroundAfter->SetOpacity(0.f);
		}
	}
	else {
		mFade_t -= DeltaTime() * mFadeSpeed;
		if (mFade_t <= 0) {
			mFade_t = 0.f;
			mBackground->Remove();
			mBackground = mBackgroundAfter;
			mBackground->SetOpacity(1.f);
			mBackgroundAfter = nullptr;
		}
		else {
			mBackground->SetOpacity(mFade_t);
			mBackgroundAfter->SetOpacity(1 - mFade_t);
		}
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

std::string Script_LoginUI::GetNextBackgroundIdx()
{
	++mCurBackgroundIdx;
	if (mCurBackgroundIdx >= 7) {
		mCurBackgroundIdx = 0;
	}

	return std::to_string(mCurBackgroundIdx);
}