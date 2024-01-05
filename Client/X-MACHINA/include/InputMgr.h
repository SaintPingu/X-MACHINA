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

	Vec2 mMousePos{};		// ���� ���콺 ��ġ
	Vec2 mMousePrevPos{};	// ���� ���콺 ��ġ
	Vec2 mMouseDir{};		// ����->������ ���콺 �̵� ����

private:
	InputMgr();
	virtual ~InputMgr() = default;

public:
	KeyState GetKeyState(int key) const { return mKeys.at(key).State; }
	Vec2 GetMousePos() const			{ return mMousePos; }
	Vec2 GetMouseDir() const			{ return mMouseDir; }
	Vec2 GetMouseDelta() const			{ return Vec2(mMousePos.x - mMousePrevPos.x, mMousePos.y - mMousePrevPos.y); }

public:

	// ����� Ű���� �����Ѵ�.
	void Init();

	// �� Ű���� ����(KeyInfo)�� ������Ʈ�Ѵ�.
	void Update();

	void ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK ProcessWndMsg(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam);
};
#pragma endregion
