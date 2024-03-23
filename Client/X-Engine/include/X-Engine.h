#pragma once

#define engine Engine::Inst()

class Engine : public Singleton<Engine> {
	friend Singleton;

private:
	std::wstring mTitle{};	// 윈도우 타이틀 문자열

private:
	Engine();
	virtual ~Engine() = default;

public:
	void Init(HINSTANCE hInstance, HWND hWnd, short width, short height);
	void Release();

	// call per once frame
	void Update();

	void ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void BuildObjects();
	void ReleaseObjects();
};
