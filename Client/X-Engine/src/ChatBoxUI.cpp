#include "EnginePch.h"
#include "ChatBoxUI.h"

#include "TextMgr.h"
#include "DXGIMgr.h"
#include "Timer.h"

#include "Component/UI.h"
#include "Component/Transform.h"


ChatBoxUI::ChatBoxUI(const Vec2& position, const Vec2& extent, const std::string& chatName)
	: 
	mChatName(StringToWstring(chatName))
{
	// ChatTitle
	{
		TextOption textOption;
		textOption.FontSize   = 30.f;
		textOption.FontWeight = TextFontWeight::HEAVY;
		textOption.HAlignment = TextAlignType::Justified;
		textOption.VAlignment = TextParagraphAlign::Near;
		textOption.BoxExtent  = extent;

		mChatTitle = TextMgr::I->CreateText("CHATTING", Vec2(position.x, position.y + 100), textOption);
	}

	// ChatMain
	{
		TextOption textOption;
		textOption.FontSize = 20.f;
		textOption.FontWeight = TextFontWeight::LIGHT;
		textOption.HAlignment = TextAlignType::Leading;
		textOption.VAlignment = TextParagraphAlign::Far;
		textOption.BoxExtent = extent;

		mChat = TextMgr::I->CreateText("", position, textOption);
	}

	mBackground = Canvas::I->CreateUI<UI>(0, "Black", Vec2{ position.x, position.y }, extent.x, extent.y);
	mBackground->mObjectCB.AlphaIntensity = 0.2f;
}

void ChatBoxUI::SetPosition(const Vec2& position)
{
	mChatTitle->SetPosition(position);
	mChat->SetPosition(position);
}

void ChatBoxUI::ToggleChatBox()
{
	if (!mIsActive) {
		mIsActive = true;
		mChat->SetAlpha(1.f);
		mBackground->mObjectCB.AlphaIntensity = 0.5f;
	}
	else {
		if (mIsEditing) {
			if (!mTotalText.empty()) {
				if (mTotalText.back() == '\n') {
					mIsEditing = false;
				}
				else {
					if (ChatCommand()) {
						return;
					}
					mTotalText.insert(mLastChatIdx, L'[' + mChatName + L"] ");
					mCurrChatCnt++;
					mTotalText += L'\n';
					
					mEditingText.clear();
					return;
				}
			}
		}

		mIsActive = false;
		mBackground->mObjectCB.AlphaIntensity = 0.2f;
	}
}

void ChatBoxUI::Update()
{
	if (!mIsActive) {
		mChat->AddAlpha(-0.5f * DeltaTime());

		if (mChat->GetAlpha() <= 0.f) {
			if (mCurrChatCnt > mMaxClearChat) {
				ClearChat(false);
			}
		}
	}
}

void ChatBoxUI::AddChat(const std::string& chat, const std::string& name)
{
	std::string res = '[' + name + "] " + chat + '\n';
	mTotalText.insert(mLastChatIdx, StringToWstring(res));
	mCurrChatCnt++;

	mChat->SetText(mTotalText);

	mChat->SetAlpha(1.f);
}

void ChatBoxUI::ClearChat(bool isAllClear)
{
	if (!isAllClear) {
		mLastChatIdx = 0;
		mTotalText = mEditingText;
	}
	else {
		mLastChatIdx = 0;
		mEditingText.clear();
		mTotalText.clear();
	}

	mCurrChatCnt = 0;
	mChat->SetText(mEditingText);
}

bool ChatBoxUI::ChatCommand()
{
	if (mEditingText == L"/clear") {
		ClearChat(true);
		return true;
	}

	return false;
}

void ChatBoxUI::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	mIsEditing = true;

	HIMC hIMC = ImmGetContext(DXGIMgr::I->GetHwnd());
	
	switch (messageID) {
	case WM_KEYDOWN:
	{
		if (wParam == VK_BACK && !mTotalText.empty()) {
			if (mTotalText.back() != '\n') {
				mTotalText.pop_back();
				mEditingText.pop_back();
			}
		}
	}
	break;
	case WM_CHAR: // english
	{
		if ((wParam < 0x20 || wParam > 0x7E)) {
			break;
		}
		else {
			mTotalText += static_cast<wchar_t>(wParam);
			mEditingText += static_cast<wchar_t>(wParam);
		}
	}
	break;
	case WM_IME_COMPOSITION: // korean
	{
		if (lParam & GCS_COMPSTR) {
			DWORD dwSize = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, nullptr, 0);
			std::wstring compStr(dwSize / sizeof(wchar_t), 0);
			ImmGetCompositionStringW(hIMC, GCS_COMPSTR, &compStr[0], dwSize);
			mImeCompositionString = compStr;
		}
		else if (lParam & GCS_RESULTSTR) {
			DWORD dwSize = ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, nullptr, 0);
			std::wstring resultStr(dwSize / sizeof(wchar_t), 0);
			ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, &resultStr[0], dwSize);
			mTotalText += resultStr;
			mEditingText += resultStr;
			mImeCompositionString.clear();
		}
		ImmReleaseContext(DXGIMgr::I->GetHwnd(), hIMC);
		break;
	}
	default:
		break;
	}

	mLastChatIdx = mTotalText.size() - mEditingText.size();
	mChat->SetText(mTotalText + mImeCompositionString);
}
