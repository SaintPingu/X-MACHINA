#include "EnginePch.h"
#include "Component/UI.h"

#include "DXGIMgr.h"
#include "TextMgr.h"
#include "Timer.h"

InputField::InputField(const std::string& textureName, const Vec2& pos, Vec2 scale)
	: UI(textureName, pos, scale)
{
	SetHoverable(true);

	{
		TextOption textOption;
		textOption.FontSize = scale.y;
		textOption.FontColor = TextFontColor::Type::Black;
		textOption.FontWeight = TextFontWeight::LIGHT;
		textOption.HAlignment = TextAlignType::Leading;
		textOption.VAlignment = TextParagraphAlign::Far;
		textOption.BoxExtent = scale;

		mTextBox = TextMgr::I->CreateText("", pos, textOption);
	}
}

std::string InputField::GetText()
{
	return WstringToString(mText + mImeCompositionString);
}

void InputField::Update()
{
	base::Update();

	if (mClicked) {
		mCurBlinkDelay += DeltaTime();
		if (mCurBlinkDelay >= mkMaxBlinkDelay) {
			mCurBlinkDelay = 0.f;
			mIsBlink = !mIsBlink;
		}
	}

	UpdateText();
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
		switch (wParam) {
		case VK_BACK:
			if (!mText.empty()) {
				mText.pop_back();
			}
			break;
		case VK_RETURN:
		case VK_ESCAPE:
			OffClick();
			break;
		}
	}
	break;
	case WM_CHAR: // english
	{
		if ((wParam < 0x20 || wParam > 0x7E)) {
			break;
		}
		else {
			AddText(static_cast<wchar_t>(wParam));
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
			AddText(resultStr);
			mImeCompositionString.clear();
		}
		ImmReleaseContext(DXGIMgr::I->GetHwnd(), hIMC);
		break;
	}
	default:
		break;
	}

	UpdateText();
}

void InputField::UpdateText()
{
	std::wstring text = mText + mImeCompositionString;
	if (mIsSecured) {
		size_t size = text.size();
		text.clear();
		text.resize(size, '*');
	}
	if (mIsBlink) {
		text += L'|';
	}

	mTextBox->SetText(text);
}

void InputField::AddText(wchar_t text)
{
	std::wstring t{};
	t += text;
	AddText(t);
}

void InputField::AddText(const std::wstring& text)
{
	mIsBlink = true;
	mCurBlinkDelay = 0.f;
	mText += text;
}

void InputField::OnClick()
{
	base::OnClick();

	mClicked = true;
}

void InputField::OffClick()
{
	base::OffClick();

	mClicked = false;

	mIsBlink = false;
	mCurBlinkDelay = mkMaxBlinkDelay;
}