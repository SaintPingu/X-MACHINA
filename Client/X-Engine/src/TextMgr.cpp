#include "EnginePch.h"
#include "TextMgr.h"

#include "DXGIMgr.h"

sptr<TextBox> TextBox::Init(const TextOption& option)
{
	mOption = option;

	THROW_IF_FAILED(DEVICE_CONTEXT->CreateSolidColorBrush(D2D1::ColorF(option.Color), &mTextBrush));
	THROW_IF_FAILED(DWRITE->CreateTextFormat(
		AnsiToWString(option.Font).c_str(),
		nullptr,
		option.FontWeight,
		option.FontStyle,
		option.FontStretch,
		option.Size,
		L"en-us",
		&mTextFormat)
	);

	THROW_IF_FAILED(mTextFormat->SetTextAlignment(option.HAlignment));
	THROW_IF_FAILED(mTextFormat->SetParagraphAlignment(option.VAlignment));

	TextMgr::I->AddTextBox(shared_from_this());

	return shared_from_this();
}

Vec2 TextBox::GetPosition() const
{
	return Vec2{ mMtxTransition._31, mMtxTransition._32 };
}

void TextBox::SetPosition(float x, float y)
{
	mMtxTransition = D2D1::Matrix3x2F::Translation(x, y);

	SetFinalMatrix();
}

void TextBox::SetRotation(float angle)
{
	mMtxRotation = D2D1::Matrix3x2F::Rotation(angle, GetTextCenter());

	SetFinalMatrix();
}

void TextBox::SetScale(float x, float y)
{
	mMtxScale = D2D1::Matrix3x2F::Scale(D2D1_SIZE_F{ x, y }, GetTextCenter());

	SetFinalMatrix();
}

void TextBox::SetFinalMatrix()
{
	mMtxFinal.SetProduct(mMtxScale, mMtxRotation);
	mMtxFinal.SetProduct(mMtxFinal, mMtxTransition);
}

void TextBox::SetColor(D2D1::ColorF color)
{
	mTextBrush->SetColor(color);
}

void TextBox::WriteText(const std::string& text)
{
	mText = AnsiToWString(text);
}

void TextBox::AddAlpha(float alpha)
{
	float originAlpha = mTextBrush->GetOpacity();
	float newAlpha = originAlpha + alpha;

	if (newAlpha <= 0.f) {
		newAlpha = 0.f;
	}
	else if (newAlpha >= 1.f) {
		newAlpha = 1.f;
	}

	mTextBrush->SetOpacity(newAlpha);
}

void TextBox::Render(RComPtr<ID2D1DeviceContext2> device) const
{
	device->SetTransform(mMtxFinal);

	device->DrawText(
		mText.c_str(),
		static_cast<UINT>(mText.length()),
		mTextFormat.Get(),
		&mOption.Rect,
		mTextBrush.Get()
	);
}

void TextBox::Destroy()
{
	TextMgr::I->RemoveTextBox(shared_from_this());
}

D2D1_POINT_2F TextBox::GetTextCenter() const
{
	const float width = mOption.Rect.left / 2.f;
	const float height = mOption.Rect.top / 2.f;

	const Vec2 center = Vec2{ width, height } - GetPosition();

	return D2D1_POINT_2F{ center.x, center.y };
}

void TextMgr::Render(RComPtr<ID2D1DeviceContext2> device)
{
	for (const auto& textBox : mTextBoxes) {
		textBox->Render(device);
	}
}
