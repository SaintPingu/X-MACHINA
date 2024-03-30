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
	Vec2 mMousePos{};		// ���� ���콺 ��ġ
	Vec2 mMouseDir{};		// ����->������ ���콺 �̵� ����

private:
	InputMgr();
	virtual ~InputMgr() = default;

public:
	KeyState GetKeyState(int key) const { return mKeys.at(key).State; }
	Vec2 GetMousePos() const			{ return mMousePos; }
	Vec2 GetMouseDir() const			{ return mMouseDir; }
	Vec2 GetMouseDelta() const			{ return Vec2(mMousePos.x - mClientCenter.x, mMousePos.y - mClientCenter.y); }
public:

	// ����� Ű���� �����Ѵ�.
	void Init();

	void UpdateClient();

	// �� Ű���� ����(KeyInfo)�� ������Ʈ�Ѵ�.
	void Update();

	void WindowFocusOn() const;
	void WindowFocusOff();

private:
	void SetCursorCenter() const;
};
#pragma endregion
