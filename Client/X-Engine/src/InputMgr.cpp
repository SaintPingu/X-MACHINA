#include "EnginePch.h"
#include "InputMgr.h"
#include "DXGIMgr.h"

#include "X-Engine.h"
#include "Timer.h"
#include "BattleScene.h"

#include "../Imgui/ImGuiMgr.h"


InputMgr::InputMgr()
{
}


void InputMgr::Init()
{
	std::wcout.imbue(std::locale("korean"));

	// ����� Ű�� ���ss
	constexpr int kKeyList[] =
	{
		// KEYBOARD //
		VK_ESCAPE, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
		'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '+',
		VK_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']',
		'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', VK_RETURN,
		VK_SHIFT, 'Z', 'X', 'C', 'V', 'B', 'N', 'M',
		VK_CONTROL, VK_LMENU, VK_SPACE, VK_RMENU, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,

		// NUM_PAD //
		VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9,
		VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6,
		VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3,
		VK_NUMPAD0,

		// MOSUE //
		VK_LBUTTON, VK_RBUTTON,

		// CAN'T USE BELOW : NEED DECODE //
		// VK_LCONTROL, VK_RCONTROL, VK_LSHIFT, VK_RSHIFT
	};

	// �� Ű���� ����� �ҷ��´�.
	for (int key : kKeyList) {
		mKeys.insert(std::make_pair(key, KeyState::None));
	}
}

void InputMgr::InitFocus()
{
	POINT clientCenter = mClientCenter;
	::ClientToScreen(DXGIMgr::I->GetHwnd(), &clientCenter);
	::ShowCursor(FALSE);
}

void InputMgr::UpdateClient()
{
	mClientCenter = { DXGIMgr::I->GetWindowWidth() / 2, DXGIMgr::I->GetWindowHeight() / 2 };
	mMousePos = Vec2(static_cast<float>(mClientCenter.x), static_cast<float>(mClientCenter.y));
	mMaxPos.x = (Engine::I->GetWindowWidth() - 10.f) / 2.f;
	mMaxPos.y = (Engine::I->GetWindowHeight() - 30.f) / 2.f;

	InitFocus();
}


void InputMgr::Update()
{
	while (!mTapKeys.empty()) {
		*mTapKeys.top() = KeyState::Pressed;
		mTapKeys.pop();
	}

	while (!mAwayKeys.empty()) {
		*mAwayKeys.top() = KeyState::None;
		mAwayKeys.pop();
	}

	// mouse move //
	if (!ImGuiMgr::I->IsFocused() && ::GetFocus()) {
		POINT ptMouse{};
		::GetCursorPos(&ptMouse);
		::ScreenToClient(DXGIMgr::I->GetHwnd(), &ptMouse);

		Vec2 mousePos = Vec2(static_cast<float>(ptMouse.x), static_cast<float>(ptMouse.y));
		Vec2 mouseDelta = Vec2(mousePos.x - mClientCenter.x, mClientCenter.y - mousePos.y);
		mMousePos += mouseDelta * mMouseSensitivity;
		mMousePos.x = std::clamp(mMousePos.x, -mMaxPos.x, mMaxPos.x);
		mMousePos.y = std::clamp(mMousePos.y, -mMaxPos.y, mMaxPos.y);

		mMouseDir.x = mousePos.x - mClientCenter.x;
		mMouseDir.y = -(mousePos.y - mClientCenter.y);

		SetCursorCenter();
	}
	else {
		WindowFocusOff();
	}
}


void InputMgr::WindowFocusOn() const
{
	if (!ImGuiMgr::I->IsFocused()) {
		SetCursorCenter();
	}
}

void InputMgr::WindowFocusOff()
{
	mMousePos = Vec2(static_cast<float>(mClientCenter.x), static_cast<float>(mClientCenter.y));
}


void InputMgr::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		ProcessMouseMsg(hWnd, msg, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
		ProcessKeyboardMsg(hWnd, msg, wParam, lParam);
		break;

	// �ѱ� �Է� ó��
	case WM_IME_COMPOSITION:
		ProcessKoreanKeyboardMsg(hWnd, msg, wParam, lParam);
		break;
	default:
		break;
	}
}

Vec2 InputMgr::GetMouseNDCPos() const
{
	return Vec2(mMousePos.x / (Engine::I->GetWindowWidth() * 0.5f), mMousePos.y / (Engine::I->GetWindowHeight() * 0.5f));
}

void InputMgr::SetCursorCenter() const
{
	POINT clientCenter = mClientCenter;
	::ClientToScreen(DXGIMgr::I->GetHwnd(), &clientCenter);
	::SetCursorPos(clientCenter.x, clientCenter.y);
}


void InputMgr::ProcessKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int key = static_cast<int>(wParam);
	if (mKeys.contains(key)) {
		auto& state = mKeys[key];

		if (message == WM_KEYDOWN && state == KeyState::None) {
			state = KeyState::Tap;
			mTapKeys.push(&state);
		}
		else if (message == WM_KEYUP) {
			state = KeyState::Away;
			mAwayKeys.push(&state);
		}
	}
}

void InputMgr::ProcessMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int key = -1;
	bool isDown{ false };
	switch (message)
	{
	case WM_LBUTTONDOWN:
		key = static_cast<int>(VK_LBUTTON);
		isDown = true;
		break;
	case WM_RBUTTONDOWN:
		key = static_cast<int>(VK_RBUTTON);
		isDown = true;
		break;
	case WM_LBUTTONUP:
		key = static_cast<int>(VK_LBUTTON);
		isDown = false;
		break;
	case WM_RBUTTONUP:
		key = static_cast<int>(VK_RBUTTON);
		isDown = false;
		break;

	default:
		break;
	}

	if (key != -1) {
		if (isDown) {
			mKeys[key] = KeyState::Tap;
			mTapKeys.push(&mKeys[key]);
		}
		else {
			mKeys[key] = KeyState::Away;
			mAwayKeys.push(&mKeys[key]);
		}
	}
}

void InputMgr::ProcessKoreanKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HIMC hIMC = ImmGetContext(hWnd);

	// �ѱ� ���� �Է� ����('��', '��', '��')
	if (lParam & GCS_COMPSTR) {
		LONG len = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);	// get string length
		if (len > 0) {
			wchar_t input{};
			ImmGetCompositionString(hIMC, GCS_COMPSTR, &input, len);	// get string
			wchar_t ch = GetLastHangul(input);
			char key = GetAlphabetFromHangul(ch);
			SendMessage(hWnd, WM_KEYDOWN, static_cast<WPARAM>(key), 0);	// WM_KEYUP�� �ѱ� �Է� ���¿��� �ѱ��� �ƴ� ���ĺ� key�� �Ѱܹ޴´�. -> ó�� ���ʿ�
		}
	}

	ImmReleaseContext(hWnd, hIMC);
}

wchar_t InputMgr::GetLastHangul(wchar_t hangul)
{
	// �߼�
	constexpr wchar_t* jungsung = L"�������¤äĤŤƤǤȤɤʤˤ̤ͤΤϤФѤҤ�";

	// ����
	constexpr wchar_t* jongsung = L" ������������������������������������������������������";

	wchar_t result{};
	
	if (IsSyllable(hangul)) {
		const wchar_t syllable = (hangul - L'��');

		int index = syllable % 28;			// ����
		if (index > 0) {
			result = jongsung[index];
		}
		else {
			index = (syllable / 28) % 21;	// �߼�
			result = jungsung[index];
		}
	}
	else {
		result = hangul;					// �ʼ�
	}

	// ���յ� �ڼ��� ��� ������ �ڼҸ� ����
	static const std::unordered_map<wchar_t, wchar_t> jamoSeperateMap = {
		{L'��', L'��'}, {L'��', L'��'}, {L'��', L'��'}, {L'��', L'��'}, {L'��', L'��'},
		{L'��', L'��'}, {L'��', L'��'}, {L'��', L'��'}, {L'��', L'��'}, {L'��', L'��'},
		{L'��', L'��'}, {L'��', L'��'},
		{L'��', L'��'}, {L'��', L'��'}, {L'��', L'��'}, {L'��', L'��'}, {L'��', L'��'},
		{L'��', L'��'}, {L'��', L'��'},
	};

	if (jamoSeperateMap.contains(result)) {
		result = jamoSeperateMap.at(result);
	}

	return result;
}

bool InputMgr::IsSyllable(wchar_t hangul)
{
	return (hangul >= L'��') && (hangul <= L'�R');
}

char InputMgr::GetAlphabetFromHangul(wchar_t hangul)
{
	static const std::unordered_map<wchar_t, char> koreanToAlphabetMap = {
		{L'��', 'R'}, {L'��', 'R'}, {L'��', 'S'}, {L'��', 'E'}, {L'��', 'E'},
		{L'��', 'F'}, {L'��', 'A'}, {L'��', 'Q'}, {L'��', 'Q'}, {L'��', 'T'},
		{L'��', 'T'}, {L'��', 'D'}, {L'��', 'W'}, {L'��', 'W'}, {L'��', 'C'},
		{L'��', 'Z'}, {L'��', 'X'}, {L'��', 'V'}, {L'��', 'G'}, {L'��', 'K'},
		{L'��', 'I'}, {L'��', 'J'}, {L'��', 'U'}, {L'��', 'H'}, {L'��', 'Y'},
		{L'��', 'N'}, {L'��', 'B'}, {L'��', 'M'}, {L'��', 'L'}, {L'��', 'P'},
		{L'��', 'P'}, {L'��', 'O'}, {L'��', 'O'}
	};

	if (!koreanToAlphabetMap.contains(hangul)) {
		return '\0';
	}

	return koreanToAlphabetMap.at(hangul);
}
