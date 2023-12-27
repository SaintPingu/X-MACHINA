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
	XMFLOAT2             mMousePos;
	XMFLOAT2             mMousePrevPos;
	XMFLOAT2             mMouseDir;

public:
	void Init();
	void Update();

public:
	KEY_STATE GetKeyState(int key)
	{
		return mKeys[key].mState;
	}

	XMFLOAT2 GetMousePos() const { return mMousePos; }
	XMFLOAT2 GetMouseDir() const { return mMouseDir; }

	XMFLOAT2 GetMouseDelta() const
	{
		XMFLOAT2 vDelta = XMFLOAT2(mMousePos.x - mMousePrevPos.x, mMousePos.y - mMousePrevPos.y);
		return vDelta;
	}
};


#define KEY_NONE(key)		InputMgr::Inst()->GetKeyState(key) == KEY_STATE::NONE
#define KEY_TAP(key)		InputMgr::Inst()->GetKeyState(key) == KEY_STATE::TAP
#define KEY_PRESSED(key)	InputMgr::Inst()->GetKeyState(key) == KEY_STATE::PRESSED
#define KEY_AWAY(key)		InputMgr::Inst()->GetKeyState(key) == KEY_STATE::AWAY