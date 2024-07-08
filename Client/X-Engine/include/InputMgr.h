#pragma once

#pragma region Define
#define KEY_NONE(key)		(InputMgr::I->GetKeyState(key) == KeyState::None)
#define KEY_TAP(key)		(InputMgr::I->GetKeyState(key) == KeyState::Tap)
#define KEY_PRESSED(key)	(InputMgr::I->GetKeyState(key) == KeyState::Pressed)
#define KEY_AWAY(key)		(InputMgr::I->GetKeyState(key) == KeyState::Away)
#pragma endregion



namespace GameKeyInfo
{
	enum class KEY_STATE : UINT8
	{
		TAP,     // ∏∑ ¥≠∏≤
		PRESSED, // TAP ¿Ã»ƒø°µµ ∞Ëº” ¥≠∑¡¿÷¿Ω
		AWAY,    // ∏∑ ∂æ ªÛ»≤
		NONE,    // æ»¥≠∏∞ ªÛ≈¬
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

	POINT mClientCenter{};
	Vec2 mMousePos{};		// «ˆ¿Á ∏∂øÏΩ∫ ¿ßƒ°
	Vec2 mMouseDir{};		// ¿Ã¿¸->«ˆ¿Á¿« ∏∂øÏΩ∫ ¿Ãµø πÊ«‚

private:
	InputMgr();
	virtual ~InputMgr() = default;

public:
	KeyState GetKeyState(int key) const { return mKeys.at(key); }
	Vec2 GetMousePos() const			{ return mMousePos; }
	Vec2 GetMouseDir() const			{ return mMouseDir; }
	Vec2 GetMouseDelta() const			{ return Vec2(mMousePos.x - mClientCenter.x, mClientCenter.y - mMousePos.y); }

public:
	// ªÁøÎ«“ ≈∞µÈ¿ª º≥¡§«—¥Ÿ.
	void Init();
	void InitFocus();

	void UpdateClient();

	// ∞¢ ≈∞µÈ¿« ¡§∫∏(KeyInfo)∏¶ æ˜µ•¿Ã∆Æ«—¥Ÿ.
	void Update();

	void WindowFocusOn() const;
	void WindowFocusOff();

	void WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void SetCursorCenter() const;

	void ProcessKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessKoreanKeyboardMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	// ¥‹¿œ «—±€ πÆ¿⁄¿« ∏∂¡ˆ∏∑ ¿⁄º“(√ º∫->¡ﬂº∫->¡æº∫)∏¶ π›»Ø«—¥Ÿ.
	static wchar_t GetLastHangul(wchar_t hangul);
	// «—±€ πÆ¿⁄¿« ¿Ω¿˝('∞°'~'∆R') ø©∫Œ∏¶ π›»Ø«—¥Ÿ.
	static bool IsSyllable(wchar_t hangul);
	// «—±€ ¿⁄º“∏¶ ≈∞∫∏µÂø° ¥Î¿¿«œ¥¬ æÀ∆ƒ∫™¿∏∑Œ π›»Ø«—¥Ÿ. ('§±' -> 'A')
	static char GetAlphabetFromHangul(wchar_t hangul);
};
#pragma endregion
