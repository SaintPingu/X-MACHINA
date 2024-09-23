#pragma once

class Scene;
enum class SceneType;

class Engine : public Singleton<Engine> {
	friend Singleton;

private:
	short mWindowWidth{ 600 };
	short mWindowHeight{ 800 };
	bool mIsWindowFocused{ true };
	std::wstring mTitle{};	// 윈도우 타이틀 문자열`

	bool mIsChangeScene{};
	int mNextSceneType{};
	Scene* mCrntScene{};

public:
	Engine();
	virtual ~Engine() = default;

	short GetWindowWidth() const { return mWindowWidth; }
	short GetWindowHeight() const { return mWindowHeight; }

public:
	void Init(HINSTANCE hInstance, HWND hWnd);
	void Release();

	// call per once frame
	void Update();

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void LoadScene(SceneType sceneType);

private:
	void LoadScene();
	void WindowFocusOn();
	void WindowFocusOff();
};
