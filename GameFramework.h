#pragma once

class Framework {
	SINGLETON_PATTERN(Framework)

public:
	Framework();
	~Framework();

	HWND GetHwnd() const;

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void BuildObjects();
	void ReleaseObjects();

	void FrameAdvance();
	void UpdateObjects();
	void RenderObjects();

	void UpdaetInput();
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	POINT mOldCursorPos{};

	_TCHAR mFrameRate[50]{};
};


#define framework Framework::Inst()