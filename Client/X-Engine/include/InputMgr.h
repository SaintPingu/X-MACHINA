#pragma once

#pragma region Define
#define KEY_NONE(key)		(InputMgr::I->GetKeyState(key) == KeyState::None)
#define KEY_TAP(key)		(InputMgr::I->GetKeyState(key) == KeyState::Tap)
#define KEY_PRESSED(key)	(InputMgr::I->GetKeyState(key) == KeyState::Pressed)
#define KEY_AWAY(key)		(InputMgr::I->GetKeyState(key) == KeyState::Away)
#pragma endregion


class Component;


namespace GameKeyInfo
{
	enum class KEY_STATE : UINT8
	{
		TAP,     // 막 눌림
		PRESSED, // TAP 이후에도 계속 눌려있음
		AWAY,    // 막 뗀 상황
		NONE,    // 안눌린 상태
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

	enum class MoveKey : UINT8 {
		W = 0b0000'0001,
		A = 0b0000'0010,
		S = 0b0000'0100,
		D = 0b0000'1000,

	};



}


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
	//std::unordered_set<rsptr<Component>> mComponents{};

	POINT mClientCenter{};
	Vec2 mMousePos{};				// 현재 마우스 위치
	Vec2 mMaxPos{};					// 마우스 최대 위치
	float mMouseSensitivity{ 1.f };	// 마우스 감도

private:
	InputMgr();
	virtual ~InputMgr() = default;

public:
	KeyState GetKeyState(int key) const { return mKeys.at(key); }
	const Vec2& GetMousePos() const		{ return mMousePos; }
	Vec2 GetMouseDir() const			{ return Vector2::Normalized(mMousePos); }

	Vec2 GetMouseNDCPos() const;


public:
	// 사용할 키들을 설정한다.
	void Init();
	void InitFocus();

	void UpdateClient();

	// 각 키들의 정보(KeyInfo)를 업데이트한다.
	void Update();

	void WindowFocusOn();
	void WindowFocusOff();

	void WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void RegisterKeyboardMsg(rsptr<Component> component);
	void RemoveKeyboardMsg(rsptr<Component> component);
	void ClearRegisteredKeyboardMsg();

private:
	void SetCursorCenter();

	void ProcessKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessKoreanKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	// 단일 한글 문자의 마지막 자소(초성->중성->종성)를 반환한다.
	static wchar_t GetLastHangul(wchar_t hangul);
	// 한글 문자의 음절('가'~'힣') 여부를 반환한다.
	static bool IsSyllable(wchar_t hangul);
	// 한글 자소를 키보드에 대응하는 알파벳으로 반환한다. ('ㅁ' -> 'A')
	static char GetAlphabetFromHangul(wchar_t hangul);
};
#pragma endregion
