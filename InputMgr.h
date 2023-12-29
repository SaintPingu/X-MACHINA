#pragma once
enum class KEY_STATE { NONE, TAP, PRESSED, AWAY };

struct KeyInfo {
	KEY_STATE mState{};
	bool mIsPrevPushed{};
};

class InputMgr {
	SINGLETON_PATTERN(InputMgr)

public:
	InputMgr();
	~InputMgr();

private:
	std::unordered_map<int, KeyInfo> mKeys;
	Vec2             mMousePos;
	Vec2             mMousePrevPos;
	Vec2             mMouseDir;

public:
	void Init();
	void Update();

public:
	KEY_STATE GetKeyState(int key)
	{
		return mKeys[key].mState;
	}

	Vec2 GetMousePos() const { return mMousePos; }
	Vec2 GetMouseDir() const { return mMouseDir; }

	Vec2 GetMouseDelta() const
	{
		Vec2 vDelta = Vec2(mMousePos.x - mMousePrevPos.x, mMousePos.y - mMousePrevPos.y);
		return vDelta;
	}

	void ProcessMsg(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void ProcessKeyboardMsg(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK ProcessWndMsg(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};


#define KEY_NONE(key)		InputMgr::Inst()->GetKeyState(key) == KEY_STATE::NONE
#define KEY_TAP(key)		InputMgr::Inst()->GetKeyState(key) == KEY_STATE::TAP
#define KEY_PRESSED(key)	InputMgr::Inst()->GetKeyState(key) == KEY_STATE::PRESSED
#define KEY_AWAY(key)		InputMgr::Inst()->GetKeyState(key) == KEY_STATE::AWAY