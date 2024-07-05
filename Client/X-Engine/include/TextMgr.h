#pragma once

class TextBox : public std::enable_shared_from_this<TextBox> {
private:
	std::wstring mText{};
	D2D1_RECT_F mRect{};
	ComPtr<struct ID2D1SolidColorBrush> mTextBrush{};
	ComPtr<struct IDWriteTextFormat> mTextFormat{};

public:
	sptr<TextBox> Init(float fontSize, float rectWidth, float rectHeight, std::wstring font = L"Verdana", D2D1::ColorF color = D2D1::ColorF::White);

public:
	void WriteText(const std::string& text);

	void Render(RComPtr<struct ID2D1DeviceContext2> device) const;
	void Destroy();
};

class TextMgr : public Singleton<TextMgr> {
	friend Singleton;

private:
	std::set<sptr<TextBox>> mTextBoxes{};

public:
	void AddTextBox(rsptr<TextBox> textBox) { mTextBoxes.insert(textBox); }
	void RemoveTextBox(rsptr<TextBox> textBox) { mTextBoxes.erase(textBox); }

	void Render(RComPtr<struct ID2D1DeviceContext2> device);
};

