#include "stdafx.h"
#include "GameFramework.h"
#include "DXGIMgr.h"
#include "InputMgr.h"

#include "Timer.h"
#include "Scene.h"

#pragma region  - ���繮 -
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

#pragma region Imgui - ���繮 - 
	//imgui->Init();
#pragma endregion

#pragma region Log - ���繮 -
	//LOG_MGR->Init("Sample"); // �̸��� ���� �� �ִ�. 
#pragma endregion


}

void Framework::Release()
{
	ReleaseObjects();

	timer->Destroy();
	InputMgr::Inst()->Destroy();
	scene->Release();

#pragma region Imgui,Log - ���繮 -
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

#pragma region Imgui - ���繮 - 
	//imgui->RenderPrepare();
	//imgui->Render();
#pragma endregion

	// rendering
	dxgi->Render();

	// update title with fps
	std::wstring title = mTitle + L" | FPS : " + timer->GetFrameRate();
	::SetWindowText(dxgi->GetHwnd(), title.data());
}