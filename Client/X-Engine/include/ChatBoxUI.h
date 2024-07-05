#pragma once

class TextBox;
class UI;

class ChatBoxUI {

private:
	sptr<UI> mBackground{};
	sptr<TextBox> mChatTitle{};
	sptr<TextBox> mChat{};
	
	std::deque<std::string> mTexts;
	std::wstring mTextBuffer;
	
	bool mImeActive = false;
	std::wstring mImeCompositionString = L"";

public:
	ChatBoxUI(const Vec2& position, const Vec2& extent);
	
public:
	void SetPosition(const Vec2& position);

public:
	void ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam);
};

