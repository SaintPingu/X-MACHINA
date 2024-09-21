#include "EnginePch.h"
#include "Component/UI.h"

#include "DXGIMgr.h"
#include "TextMgr.h"

InputField::InputField(const std::string& textureName, const Vec2& pos, Vec2 scale)
	: UI(textureName, pos, scale)
{
	SetHoverable(true);

	{
		TextOption textOption;
		textOption.FontSize = 20.f;
		textOption.FontColor = TextFontColor::Type::Black;
		textOption.FontWeight = TextFontWeight::LIGHT;
		textOption.HAlignment = TextAlignType::Leading;
		textOption.VAlignment = TextParagraphAlign::Far;
		textOption.BoxExtent = scale;

		mTextBox = TextMgr::I->CreateText("", pos, textOption);
	}
}

void InputField::ProcessKeyboardMsg(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!mClicked) {
		return;
	}

	HIMC hIMC = ImmGetContext(DXGIMgr::I->GetHwnd());

	switch (message) {
	case WM_KEYDOWN:
	{
		if (wParam == VK_BACK && !mText.empty()) {
			mText.pop_back();
		}
		else if (wParam == VK_RETURN) {
			OffClick();
		}
	}
	break;
	case WM_CHAR: // english
	{
		if ((wParam < 0x20 || wParam > 0x7E)) {
			break;
		}
		else {
			mText += static_cast<wchar_t>(wParam);
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
			mText += resultStr;
			mImeCompositionString.clear();
		}
		ImmReleaseContext(DXGIMgr::I->GetHwnd(), hIMC);
		break;
	}
	default:
		break;
	}

	mLastChatIdx = mText.size();
	mTextBox->SetText(mText + mImeCompositionString);
}

void InputField::OnClick()
{
	base::OnClick();
	std::cout << "ON\n";
	mClicked = true;
}

void InputField::OffClick()
{
	base::OffClick();
	std::cout << "OFF\n";
	mClicked = false;
}