#include "EnginePch.h"
#include "TextMgr.h"

#include "DXGIMgr.h"

sptr<TextBox> TextBox::Init(float fontSize, float rectWidth, float rectHeight, std::wstring font, D2D1::ColorF color)
{
	mRect = D2D1_RECT_F{ rectWidth, rectHeight };

	THROW_IF_FAILED(DEVICE_CONTEXT->CreateSolidColorBrush(D2D1::ColorF(color), &mTextBrush));
	THROW_IF_FAILED(DWRITE->CreateTextFormat(
		font.c_str(),
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		L"en-us",
		&mTextFormat)
	);

	THROW_IF_FAILED(mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
	THROW_IF_FAILED(mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

	TextMgr::I->AddTextBox(shared_from_this());

	return shared_from_this();
}

void TextBox::WriteText(const std::string& text)
{
	mText = AnsiToWString(text);
}

void TextBox::Render(RComPtr<ID2D1DeviceContext2> device) const
{
	device->SetTransform(D2D1::Matrix3x2F::Identity());
	device->DrawText(
		mText.c_str(),
		static_cast<UINT>(mText.length()),
		mTextFormat.Get(),
		&mRect,
		mTextBrush.Get()
	);
}

void TextBox::Destroy()
{
	TextMgr::I->RemoveTextBox(shared_from_this());
}

void TextMgr::Render(RComPtr<ID2D1DeviceContext2> device)
{
	for (const auto& textBox : mTextBoxes) {
		textBox->Render(device);
	}
}
