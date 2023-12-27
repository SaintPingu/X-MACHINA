//-----------------------------------------------------------------------------
// File: Framework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include "Timer.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "InputMgr.h"
#include "DXGIMgr.h"

SINGLETON_PATTERN_DEFINITION(Framework)

Framework::Framework()
{
	_tcscpy_s(mFrameRate, _T("LabProject ("));
}


Framework::~Framework()
{
}

HWND Framework::GetHwnd() const
{
	return dxgi->GetHwnd();
}

bool Framework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	dxgi->OnCreate(hInstance, hMainWnd);

	BuildObjects();

	return(true);
}

void Framework::OnDestroy()
{
	ReleaseObjects();

	Timer::Inst()->Destroy();
	InputMgr::Inst()->Destroy();
	crntScene->Destroy();

	dxgi->OnDestroy();

	framework->Destroy();
}



void Framework::BuildObjects()
{
	Timer::Inst()->Reset();

	crntScene->Start();
}

void Framework::ReleaseObjects()
{
	crntScene->ReleaseObjects();
}


void Framework::FrameAdvance()
{
	Timer::Inst()->Tick(0.f);

	UpdaetInput();

	UpdateObjects();

	RenderObjects();
}

void Framework::UpdateObjects()
{
	crntScene->Update();
}



void Framework::RenderObjects()
{
	dxgi->Render();

	// get frame rate
	Timer::Inst()->GetFrameRate(mFrameRate + 12, 37);
	::SetWindowText(dxgi->GetHwnd(), mFrameRate);
}


void Framework::UpdaetInput()
{
	InputMgr::Inst()->Update();
}

void Framework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	crntScene->ProcessInput(dxgi->GetHwnd(), mOldCursorPos);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&mOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void Framework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	crntScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
		{
			dxgi->Terminate();
			::PostQuitMessage(0);
		}
		break;
		case VK_RETURN:
			break;
		case VK_F3:
			break;
		case VK_F9:
			dxgi->ToggleFullScreen();
			break;
		case 'S':
		case 'T':
		{
			dxgi->SetDrawOption((int)wParam);
			break;
		}
		default:
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_HOME:
			Timer::Inst()->Stop();
			break;
		case VK_END:
			Timer::Inst()->Start();
			break;
		case '0':
			crntScene->BlowAllExplosiveObjects();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK Framework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			Timer::Inst()->Stop();
		else
			Timer::Inst()->Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}