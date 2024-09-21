#pragma once

class TextBox;
class UI;

class ChatBoxUI {
private:
	bool			mIsActive{};
	bool			mIsEditing{};

	UI*				mBackground{};
	TextBox*		mChat{};
	
	std::wstring	mTotalText{};
	std::wstring	mEditingText{};
	std::wstring	mImeCompositionString = L"";

	std::wstring	mChatName{ L"None" };
	std::size_t		mLastChatIdx{};

	int				mCurrChatCnt{};
	const int		mMaxClearChat = 10;

public:
	ChatBoxUI(const Vec2& position, const Vec2& extent, const std::string& chatName = "None");
	
public:
	bool IsActive() const { return mIsActive; }
	void SetPosition(const Vec2& position);
	void ToggleChatBox();
	void Update();

public:
	void AddChat(const std::string& chat, const std::string& name = "None");
	void ClearChat(bool isAllClear);
	bool ChatCommand();

public:
	void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
};

