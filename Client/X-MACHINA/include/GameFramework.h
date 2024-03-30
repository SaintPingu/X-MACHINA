#pragma once

#define framework GameFramework::Inst()

class GridObject;
class Script_Player;

class GameFramework : public Singleton<GameFramework>{
	friend Singleton;

private:
	bool mIsFocused = true;
	sptr<GridObject> mTestObject{};
	sptr<Script_Player> mPlayerScript{};

public:
	void Init(HINSTANCE hInstance, HWND hWnd, short width, short height);

	LRESULT CALLBACK ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void Update();
	void Release();

private:
	void InitPlayer();
};