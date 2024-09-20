#include "EnginePch.h"
#include "Component/UI.h"

#include "DXGIMgr.h"

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
			//Toggle();
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
	//mChat->SetText(mText + mImeCompositionString);
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
}
