#pragma once

#define framework GameFramework::Inst()

class GridObject;
class Script_Player;

class GameFramework : public Singleton<GameFramework>{
	friend Singleton;

	sptr<Script_Player> mPlayerScript{};

public:
	void Init(HINSTANCE hInstance, HWND hWnd, short width, short height);
	void ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void Update();
	void Release();

private:
	void InitPlayer();
};