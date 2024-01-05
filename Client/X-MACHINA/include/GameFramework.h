#pragma once

#define framework Framework::Inst()

class Framework : public Singleton<Framework> {
	friend class Singleton;

private:
	std::wstring mTitle{};	// ������ Ÿ��Ʋ ���ڿ�

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
