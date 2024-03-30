#pragma once

#pragma region Define
#define input				InputMgr::Inst()
#define KEY_NONE(key)		(input->GetKeyState(key) == KeyState::None)
#define KEY_TAP(key)		(input->GetKeyState(key) == KeyState::Tap)
#define KEY_PRESSED(key)	(input->GetKeyState(key) == KeyState::Pressed)
#define KEY_AWAY(key)		(input->GetKeyState(key) == KeyState::Away)
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
	friend Singleton;

private:
	bool mIsFocused{};
	std::unordered_map<int, KeyInfo> mKeys{};

	HWND mPrevFocusedHwnd{};
	POINT mClientCenter{};
	Vec2 mMousePos{};		// 현재 마우스 위치
	Vec2 mMouseDir{};		// 이전->현재의 마우스 이동 방향

private:
	InputMgr();
	virtual ~InputMgr() = default;

public:
	KeyState GetKeyState(int key) const { return mKeys.at(key).State; }
	Vec2 GetMousePos() const			{ return mMousePos; }
	Vec2 GetMouseDir() const			{ return mMouseDir; }
	Vec2 GetMouseDelta() const			{ return Vec2(mMousePos.x - mClientCenter.x, mMousePos.y - mClientCenter.y); }
public:

	// 사용할 키들을 설정한다.
	void Init();

	void UpdateClient();

	// 각 키들의 정보(KeyInfo)를 업데이트한다.
	void Update();

	void WindowFocusOn() const;
	void WindowFocusOff();

private:
	void SetCursorCenter() const;
};
#pragma endregion
