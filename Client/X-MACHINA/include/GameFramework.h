#pragma once

#include "Singleton.h"
#include "Common.h"


class GridObject;
class Script_Player;
class Script_BattleManager;
class Session;
class ClientSession;

// 해상도 ( 화면 크기 )
typedef struct _tagResolution
{
	short Width;
	short Height;

}RESOLUTION, * PRESOLUTION;

class GameFramework : public Singleton<GameFramework> {
	friend Singleton;

private:
	sptr<Script_Player> mPlayerScript{};
	GridObject* mPlayer{};
	volatile bool mIsLogin{};

private:
	RESOLUTION			mResolution{};				// 해상도	
	static HINSTANCE	mhInst;
	static HWND			mhWnd;

public:
	GameFramework();
	~GameFramework();

	RESOLUTION GetWindowResolution() const { return mResolution; }
	GridObject* GetPlayer() const { return mPlayer; }

public:
	void Init(HINSTANCE hInstance);
	LRESULT ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void Launch();
	int  GameLoop();

	void Update();
	void Release();

public:
	ATOM CreateGameClientWindow();

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
	static HWND GetHwnd() { return mhWnd; }
//private:
public:
	void ConnectServer() const;
	void DisconnectServer();

	void InitPlayer();
	void Login(int sessionID);
	void ResetPlayer();
};