#include "EnginePch.h"
#include "TextMgr.h"

#include "DXGIMgr.h"

namespace {
	inline D2D1_POINT_2F ConvertPoint(const Vec2& pos)
	{
		D2D1_POINT_2F result;
		result.x = pos.x;
		result.y = pos.y;
		return result;
	}

	inline D2D1::ColorF ConvertColor(TextFontColor color)
	{
		return D2D1::ColorF(color.r, color.g, color.b, color.a);
	}

	struct D2D_TextOption {
		std::string					Font        = "Verdana";
		float						FontSize    = 20.f;
		D2D1::ColorF				FontColor   = D2D1::ColorF::White;
		DWRITE_FONT_WEIGHT			FontWeight  = DWRITE_FONT_WEIGHT_NORMAL;
		DWRITE_FONT_STYLE			FontStyle   = DWRITE_FONT_STYLE_NORMAL;
		DWRITE_FONT_STRETCH			FontStretch = DWRITE_FONT_STRETCH_NORMAL;
		DWRITE_TEXT_ALIGNMENT		HAlignment  = DWRITE_TEXT_ALIGNMENT_CENTER;
		DWRITE_PARAGRAPH_ALIGNMENT	VAlignment  = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
		Vec2						BoxExtent   = Vec2::Zero;
	};

	inline void CopyMatrix(TextMatrix& out, const D2D1::Matrix3x2F& matrix)
	{
		std::memcpy(&out, &matrix, sizeof(D2D1::Matrix3x2F));
	}

	inline TextMatrix CopyMatrix(const D2D1::Matrix3x2F& matrix)
	{
		TextMatrix result;
		std::memcpy(&result, &matrix, sizeof(D2D1::Matrix3x2F));
		return result;
	}

	inline D2D1::Matrix3x2F CopyMatrix(const TextMatrix& matrix)
	{
		D2D1::Matrix3x2F result;
		std::memcpy(&result, &matrix, sizeof(TextMatrix));
		return result;
	}
}

TextBox::TextBox(const TextOption& option)
	:
	mOption(option)
{
	if (mOption.BoxExtent == Vec2::Zero) {
		const auto& window = DXGIMgr::I->GetWindow();
		mOption.BoxExtent = { (float)window.Width, (float)window.Height };
	}

	CreateBrush();
}

void TextBox::CreateBrush()
{
	D2D_TextOption option;
	option.Font        = mOption.Font;
	option.FontSize    = mOption.FontSize;
	option.FontColor   = ConvertColor(mOption.FontColor);
	option.FontWeight  = static_cast<DWRITE_FONT_WEIGHT>(mOption.FontWeight);
	option.FontStyle   = static_cast<DWRITE_FONT_STYLE>(mOption.FontStyle);
	option.FontStretch = static_cast<DWRITE_FONT_STRETCH>(mOption.FontStretch);
	option.HAlignment  = static_cast<DWRITE_TEXT_ALIGNMENT>(mOption.HAlignment);
	option.VAlignment  = static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(mOption.VAlignment);
	option.BoxExtent   = mOption.BoxExtent;

	THROW_IF_FAILED(DEVICE_CONTEXT->CreateSolidColorBrush(D2D1::ColorF(option.FontColor), &mTextBrush));
	THROW_IF_FAILED(DWRITE->CreateTextFormat(
		StringToWstring(option.Font).c_str(),
		nullptr,
		option.FontWeight,
		option.FontStyle,
		option.FontStretch,
		option.FontSize,
		L"ko-kr",
		&mTextFormat)
	);

	THROW_IF_FAILED(mTextFormat->SetTextAlignment(option.HAlignment));
	THROW_IF_FAILED(mTextFormat->SetParagraphAlignment(option.VAlignment));
}

Vec2 TextBox::GetTextCenter() const
{
	const float width = mOption.BoxExtent.x / 2.f;
	const float height = mOption.BoxExtent.y / 2.f;

	const Vec2 center = Vec2{ width, height } - GetPosition();

	return center;
}

Vec2 TextBox::GetPosition() const
{
	return Vec2{ mMtxTransition._31, mMtxTransition._32 };
}

float TextBox::GetAlpha() const
{
	return mTextBrush->GetOpacity();
}

void TextBox::SetPosition(const Vec2& pos)
{
	CopyMatrix(mMtxTransition, D2D1::Matrix3x2F::Translation(pos.x, -pos.y));

	SetFinalMatrix();
}

void TextBox::SetRotation(float angle)
{
	CopyMatrix(mMtxRotation, D2D1::Matrix3x2F::Rotation(angle, ConvertPoint(GetTextCenter())));

	SetFinalMatrix();
}

void TextBox::SetScale(const Vec2& scale)
{
	CopyMatrix(mMtxScale, D2D1::Matrix3x2F::Scale(D2D1_SIZE_F{ scale.x, scale.y }, ConvertPoint(GetTextCenter())));

	SetFinalMatrix();
}


void TextBox::SetFinalMatrix()
{
	mMtxFinal.SetProduct(mMtxScale, mMtxRotation);
	mMtxFinal.SetProduct(mMtxFinal, mMtxTransition);
}

void TextBox::SetColor(TextFontColor color)
{
	mTextBrush->SetColor(ConvertColor(color));
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

void TextBox::SetText(const std::string& text)
{
	mText = StringToWstring(text);
}

void TextBox::SetText(const std::wstring& text)
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

std::string TextBox::GetText() const
{
	return WstringToString(mText);
}

void TextBox::Render(RComPtr<ID2D1DeviceContext2> device) const
{
	device->SetTransform(CopyMatrix(mMtxFinal));

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

TextBox* TextMgr::CreateText(const std::string& text, const Vec2& pos, const TextOption& option)
{
	sptr<TextBox> textBox = std::make_shared<TextBox>(option);
	textBox->SetText(text);
	textBox->SetPosition(pos);
	mTextBoxes.insert(textBox);

	return textBox.get();
}

void TextMgr::RemoveTextBox(TextBox* target)
{
	std::erase_if(mTextBoxes, [target](sptr<TextBox> textBox) {
		return textBox.get() == target;
		});
}

void TextMgr::Render(RComPtr<ID2D1DeviceContext2> device)
{
	if (mIsEnable) {
		for (const auto& textBox : mTextBoxes) {
			textBox->Render(device);
		}
	}
}

void TextMgr::Reset()
{
	mIsEnable = true;
	for (const auto& textBox : mTextBoxes) {
		textBox->Reset();
	}
}

void TextMgr::Clear()
{
	mTextBoxes.clear();
}

void TextMgr::CreateBrush()
{
	for (const auto& textBox : mTextBoxes) {
		textBox->CreateBrush();
	}
}



TextMatrix::TextMatrix()
{
	std::memcpy(this, &D2D1::Matrix3x2F::Identity(), sizeof(D2D1::Matrix3x2F));
}

TextMatrix TextMatrix::Identity()
{
	return CopyMatrix(D2D1::Matrix3x2F::Identity());
}
