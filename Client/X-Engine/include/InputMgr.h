#pragma once

#pragma region Define
#define input				InputMgr::Inst()
#define KEY_NONE(key)		(input->GetKeyState(key) == KeyState::None)
#define KEY_TAP(key)		(input->GetKeyState(key) == KeyState::Tap)
#define KEY_PRESSED(key)	(input->GetKeyState(key) == KeyState::Pressed)
#define KEY_AWAY(key)		(input->GetKeyState(key) == KeyState::Away)
#pragma endregion



	enum class KEY_STATE : UINT8
	{
		TAP,     // �� ����
		PRESSED, // TAP ���Ŀ��� ��� ��������
		AWAY,    // �� �� ��Ȳ
		NONE,    // �ȴ��� ����
	};


	enum class KEY : UINT8
	{
		Q, W, E, R, T, Y, U, I, O, P,
		A, S, D, F, G, H, J, K, L,
		Z, X, C, V, B, N, M,
		LEFT, RIGHT, UP, DOWN,
		_0, _1, _2, _3, _4, _5, _6, _7, _8, _9,
		NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
		LSHFT, LALT, LCTRL, SPACE, ENTER, BACK, ESC, TAB, DEL,
		LBTN, RBTN, MBTN,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

		KEY_END,
	};


#pragma region EnumClass
enum class KeyState {
	None = 0,
	Tap,
	Pressed,
	Away
};
#pragma endregion


#pragma region Class
class InputMgr : public Singleton<InputMgr> {
	friend Singleton;

private:
	bool mIsFocused{};
	std::unordered_map<int, KeyState> mKeys{};
	std::stack<KeyState*> mTapKeys{};
	std::stack<KeyState*> mAwayKeys{};

	POINT mClientCenter{};
	Vec2 mMousePos{};		// ���� ���콺 ��ġ
	Vec2 mMouseDir{};		// ����->������ ���콺 �̵� ����

private:
	InputMgr();
	virtual ~InputMgr() = default;

public:
	KeyState GetKeyState(int key) const { return mKeys.at(key); }
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

	void WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void SetCursorCenter() const;

	void ProcessKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
#pragma endregion
