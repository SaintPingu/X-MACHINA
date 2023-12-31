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

bool Framework::Init(HINSTANCE hInstance, HWND hMainWnd)
{
	dxgi->Init(hInstance, hMainWnd);

	BuildObjects();

	return true;
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

	UpdaetInput();

	UpdateObjects();

	RenderObjects();
}

void Framework::UpdaetInput()
{
	InputMgr::Inst()->Update();
}

void Framework::UpdateObjects()
{
	scene->Update();
}

void Framework::RenderObjects()
{
	dxgi->Render();

	// show title
	std::wstring title = mTitle + L" | FPS : " + timer->GetFrameRate();
	::SetWindowText(dxgi->GetHwnd(), title.data());
}