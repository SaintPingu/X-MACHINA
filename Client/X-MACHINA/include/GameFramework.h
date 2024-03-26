#pragma once

#include "Singleton.h"
#include "Common.h"


#define framework GameFramework::Inst()

class GridObject;
class Script_Player;

// 해상도 ( 화면 크기 )
typedef struct _tagResolution
{
	LONG Width;
	LONG Height;

}RESOLUTION, * PRESOLUTION;

class GameFramework : public Singleton<GameFramework> {
	friend Singleton;

	sptr<Script_Player> mPlayerScript{};

private:
	RESOLUTION			mResolution{};				// 해상도	
	static HINSTANCE	mhInst;
	static HWND			mhWnd;

public:
	GameFramework();
	~GameFramework();

public:
	bool Init(HINSTANCE hInstance, LONG width, LONG height);
	void ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void Launch();
	int  GameLoop();

	void Update();
	void Release();

public:
	ATOM CreateGameClientWindow();

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

private:
	void InitPlayer();
};