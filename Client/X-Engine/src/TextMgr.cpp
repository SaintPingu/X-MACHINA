#include "EnginePch.h"
#include "TextMgr.h"

#include "DXGIMgr.h"

sptr<TextBox> TextBox::Init(const TextOption& option)
{
	mOption = option;

	if (mOption.BoxExtent == Vec2::Zero) {
		const auto& window = DXGIMgr::I->GetWindow();
		mOption.BoxExtent = { (float)window.Width, (float)window.Height };
	}

	CreateBrush();

	TextMgr::I->AddTextBox(shared_from_this());

	return shared_from_this();
}

void TextBox::CreateBrush()
{
	THROW_IF_FAILED(DEVICE_CONTEXT->CreateSolidColorBrush(D2D1::ColorF(mOption.FontColor), &mTextBrush));
	THROW_IF_FAILED(DWRITE->CreateTextFormat(
		AnsiToWString(mOption.Font).c_str(),
		nullptr,
		mOption.FontWeight,
		mOption.FontStyle,
		mOption.FontStretch,
		mOption.FontSize,
		L"en-us",
		&mTextFormat)
	);

	THROW_IF_FAILED(mTextFormat->SetTextAlignment(mOption.HAlignment));
	THROW_IF_FAILED(mTextFormat->SetParagraphAlignment(mOption.VAlignment));
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

void TextBox::SetAlpha(float alpha)
{
	float newAlpha = alpha;
	if (alpha > 1.f) {
		newAlpha = 1.f;
	}
	else if (alpha < 0.f) {
		newAlpha = 0.f;
	}

	mTextBrush->SetOpacity(alpha);
}

void TextBox::WriteText(const std::string& text)
{
	mText = AnsiToWString(text);
}

void TextBox::WriteText(const std::wstring& text)
{
	mText = text;
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

	const auto& window = DXGIMgr::I->GetWindow();
	D2D1_RECT_F rect = {
		window.Width / 2.f - mOption.BoxExtent.x / 2.f,
		window.Height / 2.f - mOption.BoxExtent.y / 2.f,
		window.Width / 2.f + mOption.BoxExtent.x / 2.f,
		window.Height / 2.f + mOption.BoxExtent.y / 2.f,
	};

	device->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	device->DrawText(
		mText.c_str(),
		static_cast<UINT>(mText.length()),
		mTextFormat.Get(),
		&rect,
		mTextBrush.Get()
	);

	device->PopAxisAlignedClip();
}

void TextBox::Reset()
{
	mTextBrush.Reset();
	mTextFormat.Reset();
}

void TextBox::Destroy()
{
	TextMgr::I->RemoveTextBox(shared_from_this());
}

D2D1_POINT_2F TextBox::GetTextCenter() const
{
	const float width = mOption.BoxExtent.x / 2.f;
	const float height = mOption.BoxExtent.y / 2.f;

	const Vec2 center = Vec2{ width, height } - GetPosition();

	return D2D1_POINT_2F{ center.x, center.y };
}

void TextMgr::Render(RComPtr<ID2D1DeviceContext2> device)
{
	for (const auto& textBox : mTextBoxes) {
		textBox->Render(device);
	}
}

void TextMgr::Reset()
{
	for (const auto& textBox : mTextBoxes) {
		textBox->Reset();
	}
	mTextBoxes.clear();
}

void TextMgr::CreateBrush()
{
	for (const auto& textBox : mTextBoxes) {
		textBox->CreateBrush();
	}
}
