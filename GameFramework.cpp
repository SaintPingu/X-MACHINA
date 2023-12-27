//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include "Timer.h"
#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "InputMgr.h"
#include "DXGIMgr.h"

SINGLETON_PATTERN_DEFINITION(CGameFramework)

CGameFramework::CGameFramework()
{
	_tcscpy_s(mFrameRate, _T("LabProject ("));
}


CGameFramework::~CGameFramework()
{
}

HWND CGameFramework::GetHwnd() const
{
	return dxgi->GetHwnd();
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	dxgi->OnCreate(hInstance, hMainWnd);

	BuildObjects();

	return(true);
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();

	CTimer::Inst()->Destroy();
	InputMgr::Inst()->Destroy();
	crntScene->Destroy();

	dxgi->OnDestroy();

	framework->Destroy();
}



void CGameFramework::BuildObjects()
{
	CTimer::Inst()->Reset();

	crntScene->Start();
}

void CGameFramework::ReleaseObjects()
{
	crntScene->ReleaseObjects();
}


void CGameFramework::FrameAdvance()
{
	CTimer::Inst()->Tick(0.f);

	UpdaetInput();

	UpdateObjects();

	RenderObjects();
}

void CGameFramework::UpdateObjects()
{
	crntScene->Update();
}



void CGameFramework::RenderObjects()
{
	dxgi->Render();

	// get frame rate
	CTimer::Inst()->GetFrameRate(mFrameRate + 12, 37);
	::SetWindowText(dxgi->GetHwnd(), mFrameRate);
}


void CGameFramework::UpdaetInput()
{
	InputMgr::Inst()->Update();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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
			CTimer::Inst()->Stop();
			break;
		case VK_END:
			CTimer::Inst()->Start();
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

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			CTimer::Inst()->Stop();
		else
			CTimer::Inst()->Start();
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