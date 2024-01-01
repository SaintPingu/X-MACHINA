//-----------------------------------------------------------------------------
// File: Framework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include "DXGIMgr.h"
#include "InputMgr.h"

#include "Timer.h"
#include "Scene.h"

SINGLETON_PATTERN_DEFINITION(Framework)

Framework::Framework()
{
	mTitle = _T("LabProject");
}

void Framework::Init(HINSTANCE hInstance, HWND hMainWnd)
{
	dxgi->Init(hInstance, hMainWnd);

	BuildObjects();
}

void Framework::Release()
{
	ReleaseObjects();

	timer->Destroy();
	InputMgr::Inst()->Destroy();
	scene->Release();

	dxgi->Release();

	Destroy();
}



void Framework::BuildObjects()
{
	timer->Reset();

	scene->Start();
}

void Framework::ReleaseObjects()
{
	scene->ReleaseObjects();
}


void Framework::FrameAdvance()
{
	timer->Tick(0.f);
	
	// update input
	InputMgr::Inst()->Update();

	// update scene
	scene->Update();

	// rendering
	dxgi->Render();

	// update title with fps
	std::wstring title = mTitle + L" | FPS : " + timer->GetFrameRate();
	::SetWindowText(dxgi->GetHwnd(), title.data());
}