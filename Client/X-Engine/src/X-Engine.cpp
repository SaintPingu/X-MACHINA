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
	WindowInfo windowInfo{ hWnd, width, height };
	dxgi->Init(hInstance, windowInfo);

	mPlayer = std::make_shared<GridObject>();
	scene->AddDynamicObject(mPlayer);
	BuildObjects();

#pragma region Imgui - 장재문 - 
	imgui->Init();
#pragma endregion

#pragma region Log - 장재문 -
	//LOG_MGR->Init("Sample"); // 이름을 지을 수 있다. 
#pragma endregion


}

void Engine::Release()
{
	ReleaseObjects();

	timer->Destroy();
	InputMgr::Inst()->Destroy();
	scene->Release();

#pragma region Imgui,Log - 장재문 -
	imgui->Destroy();
	//LOG_MGR->Destroy();
#pragma endregion

	dxgi->Release();

	Destroy();
}


void Engine::Update()
{
	timer->Tick(60.0f);

	// update input
	InputMgr::Inst()->Update();

	// update dxgi
	dxgi->Update();

	// update scene
	scene->Update();

	// rendering
	dxgi->Render();




	// update title with fps
	std::wstring title = mTitle + L" | FPS : " + timer->GetFrameRate();
	::SetWindowText(dxgi->GetHwnd(), title.data());
}


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void Engine::ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return;

	switch (message) {
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		InputMgr::Inst()->ProcessWndMsg(hWnd, message, wParam, lParam);
		break;
	default:
		break;
	}
}


void Engine::BuildObjects()
{
	scene->Start();
}

void Engine::ReleaseObjects()
{
	scene->ReleaseObjects();
}