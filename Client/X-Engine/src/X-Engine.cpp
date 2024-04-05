#include "EnginePch.h"
#include "X-Engine.h"

#include "EnginePch.h"
#include "DXGIMgr.h"
#include "InputMgr.h"

#include "Timer.h"
#include "Scene.h"
#include "Object.h"

#pragma region  - 장재문 -
#include "../Imgui/ImguiCode/imgui.h"
#include "../Imgui/ImguiManager.h"
#include "../Log/LogMgr.h"
#pragma endregion

Engine::Engine()
	:
	mTitle(L"LabProject")
{

}


void Engine::Init(HINSTANCE hInstance, HWND hWnd, short width, short height)
{
	input->Init();

	WindowInfo windowInfo{ hWnd, width, height };
	dxgi->Init(hInstance, windowInfo);

	mPlayer = std::make_shared<GridObject>();
	scene->AddDynamicObject(mPlayer);
	BuildObjects();

#pragma region Imgui - 장재문 - 
	imgui->Init();
#pragma endregion
#pragma region Log - 장재문 -
	//LOG_MGR->Init(""); // 이름을 지을 수 있다. 
#pragma endregion
}


void Engine::Release()
{
	ReleaseObjects();

	timer->Destroy();
	input->Destroy();
	scene->Release();

#pragma region Imgui,Log - 장재문 -
	imgui->Destroy();
	//LOG_MGR->Destroy();
#pragma endregion

	dxgi->Release();

	Sleep(100);
	Destroy();
}


void Engine::Update()
{
	// update dxgi
	dxgi->Update();

	// update scene
	scene->Update();

	// update input
	input->Update();

	// rendering
	dxgi->Render();


	// update title with fps
	std::wstring title = mTitle + L" | FPS : " + timer->GetFrameRate();
	::SetWindowText(dxgi->GetHwnd(), title.data());
}


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Engine::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// 미사용 메시지 처리x //
	switch (msg) {
	case WM_SETTEXT:
	case WM_SETCURSOR:
	case WM_GETICON:
	case WM_NCHITTEST:
	case WM_NCMOUSEMOVE:
	case 174:
		return true;

	default:
		break;
	}

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}

	if (imgui->IsFocused()) {
		ShowCursor(TRUE);
		imgui->FocusOff();
		return true;
	}

	switch (msg)
	{
	case WM_SETFOCUS:
		WindowFocusOn();
		break;
	case WM_KILLFOCUS:
		WindowFocusOff();
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if (!mIsWindowFocused) {
			::SetFocus(hWnd);
		}

	break;
	default:
		break;
	}

	if (mIsWindowFocused) {
		input->WndProc(hWnd, msg, wParam, lParam);
	}

	return false;
}

void Engine::BuildObjects()
{
	scene->Start();
}


void Engine::ReleaseObjects()
{
	scene->ReleaseObjects();
}

void Engine::WindowFocusOn()
{
	input->WindowFocusOn();
	while (ShowCursor(FALSE) >= 0);
	mIsWindowFocused = true;
}

void Engine::WindowFocusOff()
{
	input->WindowFocusOff();
	while (ShowCursor(TRUE) <= 0);
	mIsWindowFocused = false;
}
