#include "EnginePch.h"
#include "InputMgr.h"
#include "DXGIMgr.h"

#include "Timer.h"
#include "Scene.h"

#include "../Imgui/ImguiManager.h"


InputMgr::InputMgr()
{
}


void InputMgr::Init()
{
	// 사용할 키들 목록ss
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

	// 각 키들의 목록을 불러온다.
	for (int key : kKeyList) {
		mKeys.insert(std::make_pair(key, KeyState::None));
	}
}

void InputMgr::InitFocus()
{
	POINT clientCenter = mClientCenter;
	::ClientToScreen(dxgi->GetHwnd(), &clientCenter);
	::ShowCursor(FALSE);
}

void InputMgr::UpdateClient()
{
	mClientCenter = { dxgi->GetWindowWidth() / 2, dxgi->GetWindowHeight() / 2 };
	mMousePos = Vec2(static_cast<float>(mClientCenter.x), static_cast<float>(mClientCenter.y));
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
}


void InputMgr::WindowFocusOn() const
{
	if (!imgui->IsFocused()) {
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

	default:
		break;
	}
}

void InputMgr::SetCursorCenter() const
{
	POINT clientCenter = mClientCenter;
	::ClientToScreen(dxgi->GetHwnd(), &clientCenter);
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

	case WM_MOUSEMOVE:
	{
		if (!imgui->IsFocused() && ::GetFocus()) {
			POINT ptMouse{};
			::GetCursorPos(&ptMouse);
			::ScreenToClient(dxgi->GetHwnd(), &ptMouse);

			mMousePos = Vec2(static_cast<float>(ptMouse.x), static_cast<float>(ptMouse.y));
			mMouseDir.x = mMousePos.x - mClientCenter.x;
			mMouseDir.y = -(mMousePos.y - mClientCenter.y);

			SetCursorCenter();
		}
		else {
			WindowFocusOff();
		}
	}
	return;

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