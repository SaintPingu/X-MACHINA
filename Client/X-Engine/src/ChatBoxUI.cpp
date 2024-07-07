#include "EnginePch.h"
#include "ChatBoxUI.h"

#include "TextMgr.h"
#include "DXGIMgr.h"

#include "Component/UI.h"
#include "Component/Transform.h"


ChatBoxUI::ChatBoxUI(const Vec2& position, const Vec2& extent)
{
	// ChatTitle
	{
		TextOption op;
		op.FontSize = 30.f;
		op.FontWeight = DWRITE_FONT_WEIGHT_HEAVY;
		op.HAlignment = DWRITE_TEXT_ALIGNMENT_JUSTIFIED;
		op.VAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
		op.BoxExtent = extent;

		mChatTitle = std::make_shared<TextBox>()->Init(op);
		mChatTitle->WriteText("CHATTING");
		mChatTitle->SetPosition(position.x, position.y - 50.f);
	}

	// ChatMain
	{
		TextOption op;
		op.FontSize = 20.f;
		op.FontWeight = DWRITE_FONT_WEIGHT_LIGHT;
		op.HAlignment = DWRITE_TEXT_ALIGNMENT_LEADING;
		op.VAlignment = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
		op.BoxExtent = extent;

		mChat = std::make_shared<TextBox>()->Init(op);
		mChat->SetPosition(position.x, position.y);
	}

	mBackground = Canvas::I->CreateUI(0, "Black", Vec2{ position.x * 2.f, position.y * -2.f }, extent.x, extent.y);
	mBackground->mObjectCB.AlphaIntensity = 0.2f;
}

void ChatBoxUI::SetPosition(const Vec2& position)
{
	mChatTitle->SetPosition(position.x, position.y);
	mChat->SetPosition(position.x, position.y);
}

void ChatBoxUI::ToggleChatBox()
{
	if (!mIsActive) {
		mIsActive = true;
		mBackground->mObjectCB.AlphaIntensity = 0.5f;
	}
	else {
		if (mIsEditing) {
			if (!mTextBuffer.empty()) {
				if (mTextBuffer.back() == '\n') {
					mIsEditing = false;
				}
				else {
					mTextBuffer += L'\n';
					mEditingText.clear();
					return;
				}
			}
		}

		mIsActive = false;
		mBackground->mObjectCB.AlphaIntensity = 0.2f;
	}
}

void ChatBoxUI::AddChat(std::string chat)
{
	std::string res = chat + '\n';

	size_t lastChatIdx = mTextBuffer.size() - mEditingText.size();

	mTextBuffer.insert(lastChatIdx, AnsiToWString(res));
	mChat->WriteText(mTextBuffer);
}

void ChatBoxUI::ClearChat()
{
	mTextBuffer.clear();
	mImeCompositionString.clear();
}

void ChatBoxUI::ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam)
{
	mIsEditing = true;

	HIMC hIMC = ImmGetContext(DXGIMgr::I->GetHwnd());
	
	switch (messageID) {
	case WM_KEYDOWN:
	{
		if (wParam == VK_BACK && !mTextBuffer.empty()) {
			if (mTextBuffer.back() != '\n') {
				mTextBuffer.pop_back();
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
			mTextBuffer += static_cast<wchar_t>(wParam);
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
			mTextBuffer += resultStr;
			mEditingText += resultStr;
			mImeCompositionString.clear();
		}
		ImmReleaseContext(DXGIMgr::I->GetHwnd(), hIMC);
		break;
	}
	default:
		break;
	}

	mChat->WriteText(mTextBuffer + mImeCompositionString);
}
