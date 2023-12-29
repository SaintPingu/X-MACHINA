#pragma once

class Framework {
	SINGLETON_PATTERN(Framework)

public:
	Framework();
	~Framework() = default;

	bool Init(HINSTANCE hInstance, HWND hMainWnd);
	void Release();

	void BuildObjects();
	void ReleaseObjects();

	void FrameAdvance();
	void UpdaetInput();
	void UpdateObjects();
	void RenderObjects();

private:
	std::wstring mTitle{};
};


#define framework Framework::Inst()