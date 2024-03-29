#include "stdafx.h"
#include "GameFramework.h"

#pragma comment(lib, "lib/X-Engine.lib")
#include "X-Engine.h"

#include "Scene.h"
#include "Timer.h"
#include "ObjectMgr.h"
#include "Object.h"
#include "Component/Camera.h"
#include "Component/ParticleSystem.h"

#include "Script_MainCamera.h"
#include "Script_Player.h"


void GameFramework::Init(HINSTANCE hInstance, HWND hWnd, short width, short height)
{
	mainCameraObject->AddComponent<Script_MainCamera>();
	engine->Init(hInstance, hWnd, width, height);
	InitPlayer();
	objectMgr->InitObjectsScript();
}

void GameFramework::ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	engine->ProcessMessage(hWnd, message, wParam, lParam);

	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		mPlayerScript->ProcessMouseMsg(message, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		mPlayerScript->ProcessKeyboardMsg(message, wParam, lParam);
		break;

	default:
		break;
	}
}

void GameFramework::ProcessKeyboardMsg(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_HOME:
			timer->Stop();
			break;
		case VK_END:
			timer->Start();
			break;

		default:
			break;
		}
	}

	break;
	default:
		break;
	}
}

void GameFramework::Update()
{
	timer->Tick(60.0f);

	engine->Update();
	scene->UpdateObjectGrid(mTestObject.get());
}

void GameFramework::Release()
{
	engine->Release();
	objectMgr->Destroy();
	Destroy();
}

void GameFramework::InitPlayer()
{
	mTestObject = scene->Instantiate("EliteTrooper", true);
	mTestObject->SetPosition(105, 13, 105);

	sptr<GridObject> player = engine->GetPlayer();
	player->ResetCollider();
	mPlayerScript = player->AddComponent<Script_GroundPlayer>();
	player->SetModel("EliteTrooper");
	
	//player->AddComponent<ParticleSystem>()->Load("Green")->SetTarget("Humanoid_ R Hand");
	player->AddComponent<ParticleSystem>()->Load("Fire")->SetTarget("Humanoid_ L Hand");
	//player->AddComponent<ParticleSystem>()->Load("Fountain")->SetTarget("Humanoid_ Head");
}

 