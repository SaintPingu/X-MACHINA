#pragma once

#define framework Framework::Inst()

class Framework : public Singleton<Framework> {
	friend class Singleton;

private:
	std::wstring mTitle{};	// 윈도우 타이틀 문자열

private:
	Framework();
	virtual ~Framework() = default;

public:
	void Init(HINSTANCE hInstance, HWND hMainWnd);
	void Release();

	void BuildObjects();
	void ReleaseObjects();

	// call per once frame
	void FrameAdvance();
};
