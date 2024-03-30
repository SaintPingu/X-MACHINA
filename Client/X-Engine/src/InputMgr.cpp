#include "EnginePch.h"
#include "InputMgr.h"
#include "DXGIMgr.h"

#include "Timer.h"
#include "Scene.h"



InputMgr::InputMgr()
{
	Init();
}


void InputMgr::Init()
{
	// 사용할 키들 목록ss
	constexpr int kKeyList[] =
	{
		VK_ESCAPE, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
		'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '+',
		VK_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']',
		'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', VK_RETURN,
		VK_LSHIFT, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', VK_RSHIFT, VK_SHIFT,
		VK_LCONTROL, VK_LMENU, VK_SPACE, VK_RMENU, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,

		VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9,
		VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6,
		VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3,
		VK_NUMPAD0,

		VK_LBUTTON, VK_RBUTTON, VK_MBUTTON,
	};

	// 각 키들의 목록을 불러온다.
	for (int key : kKeyList) {
		mKeys[key] = KeyInfo{ KeyState::None, false };
	}
}

void InputMgr::UpdateClient()
{
	mClientCenter = { dxgi->GetWindowWidth() / 2, dxgi->GetWindowHeight() / 2 };
}

void InputMgr::Update()
{
	const HWND hFocusedHwnd = ::GetFocus();

	// 윈도우가 포커싱 되어 있다면, 각 키들의 정보를 업데이트한다.
	if (hFocusedHwnd) {

		for(auto& keyInfo : mKeys) {
			int key = keyInfo.first;
			KeyInfo& info = keyInfo.second;

			// 키가 눌려있다
			if (GetAsyncKeyState(key) & 0x8000) {
				// 이전엔 눌린적이 없다.
				if (info.IsPrevPressed == false) {
					info.State = KeyState::Tap;
				}
				else {
					info.State = KeyState::Pressed;
				}
				info.IsPrevPressed = true;
			}
			// 키가 안눌려있다.
			else {
				// 이전엔 눌려있었다.
				if (info.State == KeyState::Tap || info.State == KeyState::Pressed) {
					info.State = KeyState::Away;
				}
				else if (KeyState::Away == info.State) {
					info.State = KeyState::None;
				}
				info.IsPrevPressed = false;
			}
		}

		POINT ptMouse{};
		::GetCursorPos(&ptMouse);
		::ScreenToClient(dxgi->GetHwnd(), &ptMouse);

		mMousePos     = Vec2(static_cast<float>(ptMouse.x), static_cast<float>(ptMouse.y));
		mMouseDir.x   = mMousePos.x - mClientCenter.x;
		mMouseDir.y   = -(mMousePos.y - mClientCenter.y);

		SetCursorCenter();
	}
	// 윈도우가 포커싱 되어있지 않다면, 각 키들의 상태를 away or none으로 변경한다.
	else {
		for (auto& keyInfo : mKeys) {
			KeyInfo& info = keyInfo.second;

			info.IsPrevPressed = false;
			if (KeyState::Tap == info.State || KeyState::Pressed == info.State) {
				info.State = KeyState::Away;
			}
			else if (KeyState::Away == info.State) {
				info.State = KeyState::None;
			}
		}
	}

	mPrevFocusedHwnd = hFocusedHwnd;
}


void InputMgr::WindowFocusOn() const
{
	SetCursorCenter();
}

void InputMgr::WindowFocusOff()
{
	mMousePos = Vec2(static_cast<float>(mClientCenter.x), static_cast<float>(mClientCenter.y));
}

void InputMgr::SetCursorCenter() const
{
	POINT clientCenter = mClientCenter;
	::ClientToScreen(dxgi->GetHwnd(), &clientCenter);
	::SetCursorPos(clientCenter.x, clientCenter.y);
}
