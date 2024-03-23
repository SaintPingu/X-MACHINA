#pragma once

#define engine Engine::Inst()

class GridObject;

class Engine : public Singleton<Engine> {
	friend Singleton;

private:
	std::wstring mTitle{};	// 윈도우 타이틀 문자열
	sptr<GridObject> mPlayer{};

public:
	Engine();
	virtual ~Engine() = default;

	sptr<GridObject> GetPlayer() const { return mPlayer; }

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
