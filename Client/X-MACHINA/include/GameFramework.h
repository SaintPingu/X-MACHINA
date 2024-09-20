#pragma once

#include "Singleton.h"
#include "Common.h"


class GridObject;
class Script_PlayerController;
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
	Script_PlayerController* mPlayerScript{};
	GridObject* mPlayer{};
	volatile bool mIsLogin{};
	UINT mPlayerID{};

private:
	RESOLUTION			mResolution{};				// 해상도	
	static HINSTANCE	mhInst;
	static HWND			mhWnd;


public:
	GameFramework();
	~GameFramework();

	RESOLUTION GetWindowResolution() const { return mResolution; }
	Vec2 GetWindowSize() const { return Vec2(static_cast<float>(mResolution.Width), static_cast<float>(mResolution.Height)); }
	UINT GetMyPlayerID() const { return mPlayerID; }
	GridObject* GetPlayer() const { return mPlayer; }
	Script_PlayerController* GetPlayerScript() const { return mPlayerScript; }

	void SetPlayer(GridObject* player) { mPlayer = player; }
	void SetPlayerScript(Script_PlayerController* script) { mPlayerScript = script; }

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

	void CreatePlayer();
	void Login(int sessionID);
	void ResetPlayer();
};