#pragma once

class Framework {
	SINGLETON_PATTERN(Framework)

private:
	std::wstring mTitle{};

public:
	Framework();
	virtual ~Framework() = default;

public:
	bool Init(HINSTANCE hInstance, HWND hMainWnd);
	void Release();

	void BuildObjects();
	void ReleaseObjects();

	void FrameAdvance();
	void UpdaetInput();
	void UpdateObjects();
	void RenderObjects();
};


#define framework Framework::Inst()