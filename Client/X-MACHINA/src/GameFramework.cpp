#include "stdafx.h"
#include "GameFramework.h"

#include "Resource.h"
#include "X-Engine.h"

#include "InputMgr.h"
#include "Imgui/ImGuiMgr.h"

#include "Scene.h"
#include "Timer.h"
#include "Object.h"
#include "Animator.h"
#include "AnimatorController.h"

#include "ObjectPool.h"

#include "Component/Camera.h"
#include "Component/ParticleSystem.h"


#include "Script_MainCamera.h"
#include "Script_Player.h"
#include "Script_GameManager.h"


#include "InputMgr.h"
#include "X-Engine.h"
#include "XLManager.h"

#include "ClientNetwork/Contents/ClientNetworkManager.h"
#include "ClientNetwork/Include/ThreadManager.h"
#include "ClientNetwork/Contents/Script_PlayerNetwork.h"

//#define SERVER_COMMUNICATION


HINSTANCE GameFramework::mhInst = nullptr;
HWND      GameFramework::mhWnd = nullptr;


GameFramework::GameFramework()
{

}

GameFramework::~GameFramework()
{
}

void GameFramework::Init(HINSTANCE hInstance, short width, short height)
{
	/* mhWnd 초기화 */
	mhInst = hInstance;
	mResolution.Width = width;
	mResolution.Height = height;
	CreateGameClientWindow();

	// Init //
	Engine::I->Init(hInstance, mhWnd, static_cast<short>(width), static_cast<short>(height));

	XLManger::I->LoadTables();
}

void GameFramework::Release()
{
}




int GameFramework::GameLoop()
{
	static HACCEL hAccelTable = LoadAccelerators(mhInst, MAKEINTRESOURCE(IDC_XMACHINA));
	static MSG msg{};

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue; // ImGui 

			}
		}
		else
		{
			GameFramework::I->Update();
		}
	}

	Engine::I->Release();
	::DestroyWindow(mhWnd);
	::UnregisterClass(L"X-MACHINA", mhInst);

	return (int)msg.wParam;
}


void GameFramework::Update()
{
	Timer::I->Tick(0.f);

#ifdef SERVER_COMMUNICATION
	CLIENT_NETWORK->ProcessEvents();

#endif

	Engine::I->Update();
}



void GameFramework::Launch()
{
	ConnectToServer();

	/* main Thread */
	GameLoop();

#ifdef SERVER_COMMUNICATION
	THREAD_MGR->JoinAllThreads();
#endif
}



LRESULT GameFramework::ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (Engine::I->WndProc(hWnd, message, wParam, lParam)) {
		return 0;
	}

	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		ProcessMouseMsg(hWnd, message, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		ProcessKeyboardMsg(hWnd, message, wParam, lParam);
		break;

	default:
		break;
	}

	return 0;
}

void GameFramework::ProcessKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_F1:
			Timer::I->Stop();
			break;
		case VK_F2:
			Timer::I->Start();
			break;
		case VK_F5:
			Scene::I->ToggleDrawBoundings();
			break;
		case VK_F6:
			ImGuiMgr::I->ToggleImGui();
			break;
		case VK_F8:
			Scene::I->ToggleFilterOptions();
			break;
		case VK_F9:
			Scene::I->ToggleFullScreen();
			break;
		case 192:	// '`'
			::SetFocus(NULL);
			break;

		default:
			break;
		}
	}

	break;
	default:
		break;
	}

	if (const auto& playerScript = mPlayerScript.lock()) {
		playerScript->ProcessKeyboardMsg(message, wParam, lParam);
	}
}

void GameFramework::ProcessMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (const auto& playerScript = mPlayerScript.lock()) {
		playerScript->ProcessMouseMsg(message, wParam, lParam);
	}
}

ATOM GameFramework::CreateGameClientWindow()
{
	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = mhInst;
	wcex.hIcon = LoadIcon(mhInst, MAKEINTRESOURCE(IDI_XMACHINA));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCEW(IDC_CLIENT);
	wcex.lpszClassName = L"X-MACHINA";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassExW(&wcex);

	RECT rc = { 0, 0, mResolution.Width, mResolution.Height };
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_BORDER;
	AdjustWindowRect(&rc, dwStyle, FALSE);

	int adjustedWidth = rc.right - rc.left;
	int adjustedHeight = rc.bottom - rc.top;

	mhWnd = ::CreateWindow(L"X-MACHINA", L"X-MACHINA", dwStyle
		, CW_USEDEFAULT, 0, adjustedWidth, adjustedHeight
		, nullptr, nullptr, mhInst, nullptr);


	::ShowWindow(mhWnd, SW_SHOWDEFAULT);
	::UpdateWindow(mhWnd);

	return ::RegisterClassEx(&wcex);

}

LRESULT GameFramework::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GameFramework::I->ProcessMessage(hWnd, message, wParam, lParam)) {
		return 0;
	}

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			::DialogBox(mhInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		default:
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR GameFramework::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void GameFramework::ConnectToServer()
{
	MainCamera::I->AddComponent<Script_MainCamera>();

#ifdef SERVER_COMMUNICATION
	// Communication //
	//std::cout << "IP : ";
	std::wstring ip;
	//std::wcin >> ip;
	CLIENT_NETWORK->Init(ip, 7777);

	/* Network Thread */
	CLIENT_NETWORK->Launch(2);

	while (!mIsLogin) {
		CLIENT_NETWORK->ProcessEvents();
	}
#else
	InitPlayer(0);

#endif

	mGameManager = GAME_MGR->AddComponent<Script_GameManager>();
}



void GameFramework::InitPlayer(int sessionID)
{
	mPlayer = Scene::I->Instantiate("EliteTrooper", ObjectTag::Player);
	mPlayer->ResetCollider();
	mPlayerScript = mPlayer->AddComponent<Script_PheroPlayer>();

#ifdef SERVER_COMMUNICATION
	auto& networkScript = mPlayer->AddComponent<Script_PlayerNetwork>();
#endif

	mIsLogin = true;
}