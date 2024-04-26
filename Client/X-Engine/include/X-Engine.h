#pragma once

class GridObject;

class Engine : public Singleton<Engine> {
	friend Singleton;

private:
	bool mIsWindowFocused{ true };
	std::wstring mTitle{};	// 윈도우 타이틀 문자열

public:
	Engine();
	virtual ~Engine() = default;

public:
	void Init(HINSTANCE hInstance, HWND hWnd, short width, short height);
	void Release();

	// call per once frame
	void Update();

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void BuildObjects();

	void WindowFocusOn();
	void WindowFocusOff();
};
