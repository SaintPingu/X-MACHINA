#pragma once

#pragma region Define
#define KEY_NONE(key)		InputMgr::Inst()->GetKeyState(key) == KeyState::None
#define KEY_TAP(key)		InputMgr::Inst()->GetKeyState(key) == KeyState::Tap
#define KEY_PRESSED(key)	InputMgr::Inst()->GetKeyState(key) == KeyState::Pressed
#define KEY_AWAY(key)		InputMgr::Inst()->GetKeyState(key) == KeyState::Away
#pragma endregion


#pragma region EnumClass
enum class KeyState {
	None = 0,
	Tap,
	Pressed,
	Away
};
#pragma endregion


#pragma region Struct
struct KeyInfo {
	KeyState	State{};
	bool		IsPrevPressed{};
};
#pragma endregion


#pragma region Class
class InputMgr : public Singleton<InputMgr> {
	friend class Singleton;

private:
	std::unordered_map<int, KeyInfo> mKeys{};

	Vec2 mMousePos{};		// 현재 마우스 위치
	Vec2 mMousePrevPos{};	// 이전 마우스 위치
	Vec2 mMouseDir{};		// 이전->현재의 마우스 이동 방향

private:
	InputMgr();
	virtual ~InputMgr() = default;

public:
	KeyState GetKeyState(int key) const { return mKeys.at(key).State; }
	Vec2 GetMousePos() const			{ return mMousePos; }
	Vec2 GetMouseDir() const			{ return mMouseDir; }
	Vec2 GetMouseDelta() const			{ return Vec2(mMousePos.x - mMousePrevPos.x, mMousePos.y - mMousePrevPos.y); }

public:

	// 사용할 키들을 설정한다.
	void Init();

	// 각 키들의 정보(KeyInfo)를 업데이트한다.
	void Update();

	void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK ProcessWndMsg(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam);
};
#pragma endregion
