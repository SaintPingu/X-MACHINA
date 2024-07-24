#pragma once

struct TextOption {
	std::string					Font = "Verdana";
	float						FontSize = 20.f;
	D2D1::ColorF				FontColor = D2D1::ColorF::White;
	DWRITE_FONT_WEIGHT			FontWeight = DWRITE_FONT_WEIGHT_NORMAL;
	DWRITE_FONT_STYLE			FontStyle = DWRITE_FONT_STYLE_NORMAL;
	DWRITE_FONT_STRETCH			FontStretch = DWRITE_FONT_STRETCH_NORMAL;
	DWRITE_TEXT_ALIGNMENT		HAlignment = DWRITE_TEXT_ALIGNMENT_CENTER;
	DWRITE_PARAGRAPH_ALIGNMENT	VAlignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
	Vec2						BoxExtent = Vec2::Zero;
};

class TextBox : public std::enable_shared_from_this<TextBox> {
private:
	TextOption mOption{};

	ComPtr<struct ID2D1SolidColorBrush> mTextBrush{};
	ComPtr<struct IDWriteTextFormat> mTextFormat{};

	std::wstring mText{};

	D2D1::Matrix3x2F mMtxFinal = D2D1::Matrix3x2F::Identity();
	D2D1::Matrix3x2F mMtxScale = D2D1::Matrix3x2F::Identity();
	D2D1::Matrix3x2F mMtxRotation = D2D1::Matrix3x2F::Identity();
	D2D1::Matrix3x2F mMtxTransition = D2D1::Matrix3x2F::Identity();

public:
	sptr<TextBox> Init(const TextOption& option = TextOption{});
	void CreateBrush();

public:
	Vec2 GetPosition() const;
	float GetAlpha() const { return mTextBrush->GetOpacity(); }

public:
	void SetPosition(float x, float y);
	void SetRotation(float angle);
	void SetScale(float x, float y);
	void SetColor(D2D1::ColorF color = D2D1::ColorF(D2D1::ColorF::White));
	void SetAlpha(float alpha);

public:	
	void WriteText(const std::string& text);
	void WriteText(const std::wstring& text);
	void AddAlpha(float alpha);

public:
	void Render(RComPtr<struct ID2D1DeviceContext2> device) const;
	void Reset();
	void Destroy();

private:
	void SetFinalMatrix();
	D2D1_POINT_2F GetTextCenter() const;
};

class TextMgr : public Singleton<TextMgr> {
	friend Singleton;

private:
	std::set<sptr<TextBox>> mTextBoxes{};

public:
	void AddTextBox(rsptr<TextBox> textBox) { mTextBoxes.insert(textBox); }
	void RemoveTextBox(rsptr<TextBox> textBox) { mTextBoxes.erase(textBox); }

	void Render(RComPtr<struct ID2D1DeviceContext2> device);
	void Reset();
	void CreateBrush();
};
