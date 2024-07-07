#pragma once

class TextBox;
class UI;

class ChatBoxUI {
private:
	bool			mIsActive{};
	bool			mIsEditing{};
	sptr<UI>		mBackground{};
	sptr<TextBox>	mChatTitle{};
	sptr<TextBox>	mChat{};
	
	std::deque<std::wstring>	mTexts;
	std::wstring				mTextBuffer;
	std::wstring				mImeCompositionString = L"";


public:
	ChatBoxUI(const Vec2& position, const Vec2& extent);
	
public:
	bool IsActive() const { return mIsActive; }
	void SetPosition(const Vec2& position);
	void ToggleChatBox();

public:
	void AddChat(std::string chat);
	void ClearChat();

public:
	void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
};

