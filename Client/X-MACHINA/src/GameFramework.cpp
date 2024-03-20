#include "stdafx.h"
#include "GameFramework.h"
#include "DXGIMgr.h"
#include "InputMgr.h"

#include "Timer.h"
#include "Scene.h"

#pragma region  - 장재문 -
#include "../Imgui/ImguiMgr.h"
#include "../Log/LogMgr.h"
#pragma endregion



Framework::Framework()
	:
	mTitle(L"LabProject")
{

}

void Framework::Init(HINSTANCE hInstance, HWND hMainWnd)
{
	dxgi->Init(hInstance, hMainWnd);

	BuildObjects();

#pragma region Imgui - 장재문 - 
	//imgui->Init();
#pragma endregion

#pragma region Log - 장재문 -
	//LOG_MGR->Init("Sample"); // 이름을 지을 수 있다. 
#pragma endregion


}

void Framework::Release()
{
	ReleaseObjects();

	timer->Destroy();
	InputMgr::Inst()->Destroy();
	scene->Release();

#pragma region Imgui,Log - 장재문 -
	//imgui->Destroy();
	//LOG_MGR->Destroy();
#pragma endregion

	dxgi->Release();

	Destroy();
}



void Framework::BuildObjects()
{
	scene->Start();
}

void Framework::ReleaseObjects()
{
	scene->ReleaseObjects();
}


void Framework::FrameAdvance()
{
	timer->Tick(60.0f);
	
	// update input
	InputMgr::Inst()->Update();

	// update dxgi
	dxgi->Update();

	// update scene
	scene->Update();

#pragma region Imgui - 장재문 - 
	//imgui->RenderPrepare();
	//imgui->Render();
#pragma endregion

	// rendering
	dxgi->Render();

	// update title with fps
	std::wstring title = mTitle + L" | FPS : " + timer->GetFrameRate();
	::SetWindowText(dxgi->GetHwnd(), title.data());
}