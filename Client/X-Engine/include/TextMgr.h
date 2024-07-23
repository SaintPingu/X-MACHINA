#pragma once

enum class TextAlignType {
	Center, Top, Bottom
};
enum class TextFontWeight {
	THIN = 100,
	EXTRA_LIGHT = 200,
	ULTRA_LIGHT = 200,
	LIGHT = 300,
	SEMI_LIGHT = 350,
	NORMAL = 400,
	REGULAR = 400,
	MEDIUM = 500,
	DEMI_BOLD = 600,
	SEMI_BOLD = 600,
	BOLD = 700,
	EXTRA_BOLD = 800,
	ULTRA_BOLD = 800,
	BLACK = 900,
	HEAVY = 900,
	EXTRA_BLACK = 950,
	ULTRA_BLACK = 950
};
enum class TextFontStyle {
	Normal,
	Oblique,
	Italic
};
enum class TextColor {
    AliceBlue = 0xF0F8FF,
    AntiqueWhite = 0xFAEBD7,
    Aqua = 0x00FFFF,
    Aquamarine = 0x7FFFD4,
    Azure = 0xF0FFFF,
    Beige = 0xF5F5DC,
    Bisque = 0xFFE4C4,
    Black = 0x000000,
    BlanchedAlmond = 0xFFEBCD,
    Blue = 0x0000FF,
    BlueViolet = 0x8A2BE2,
    Brown = 0xA52A2A,
    BurlyWood = 0xDEB887,
    CadetBlue = 0x5F9EA0,
    Chartreuse = 0x7FFF00,
    Chocolate = 0xD2691E,
    Coral = 0xFF7F50,
    CornflowerBlue = 0x6495ED,
    Cornsilk = 0xFFF8DC,
    Crimson = 0xDC143C,
    Cyan = 0x00FFFF,
    DarkBlue = 0x00008B,
    DarkCyan = 0x008B8B,
    DarkGoldenrod = 0xB8860B,
    DarkGray = 0xA9A9A9,
    DarkGreen = 0x006400,
    DarkKhaki = 0xBDB76B,
    DarkMagenta = 0x8B008B,
    DarkOliveGreen = 0x556B2F,
    DarkOrange = 0xFF8C00,
    DarkOrchid = 0x9932CC,
    DarkRed = 0x8B0000,
    DarkSalmon = 0xE9967A,
    DarkSeaGreen = 0x8FBC8F,
    DarkSlateBlue = 0x483D8B,
    DarkSlateGray = 0x2F4F4F,
    DarkTurquoise = 0x00CED1,
    DarkViolet = 0x9400D3,
    DeepPink = 0xFF1493,
    DeepSkyBlue = 0x00BFFF,
    DimGray = 0x696969,
    DodgerBlue = 0x1E90FF,
    Firebrick = 0xB22222,
    FloralWhite = 0xFFFAF0,
    ForestGreen = 0x228B22,
    Fuchsia = 0xFF00FF,
    Gainsboro = 0xDCDCDC,
    GhostWhite = 0xF8F8FF,
    Gold = 0xFFD700,
    Goldenrod = 0xDAA520,
    Gray = 0x808080,
    Green = 0x008000,
    GreenYellow = 0xADFF2F,
    Honeydew = 0xF0FFF0,
    HotPink = 0xFF69B4,
    IndianRed = 0xCD5C5C,
    Indigo = 0x4B0082,
    Ivory = 0xFFFFF0,
    Khaki = 0xF0E68C,
    Lavender = 0xE6E6FA,
    LavenderBlush = 0xFFF0F5,
    LawnGreen = 0x7CFC00,
    LemonChiffon = 0xFFFACD,
    LightBlue = 0xADD8E6,
    LightCoral = 0xF08080,
    LightCyan = 0xE0FFFF,
    LightGoldenrodYellow = 0xFAFAD2,
    LightGreen = 0x90EE90,
    LightGray = 0xD3D3D3,
    LightPink = 0xFFB6C1,
    LightSalmon = 0xFFA07A,
    LightSeaGreen = 0x20B2AA,
    LightSkyBlue = 0x87CEFA,
    LightSlateGray = 0x778899,
    LightSteelBlue = 0xB0C4DE,
    LightYellow = 0xFFFFE0,
    Lime = 0x00FF00,
    LimeGreen = 0x32CD32,
    Linen = 0xFAF0E6,
    Magenta = 0xFF00FF,
    Maroon = 0x800000,
    MediumAquamarine = 0x66CDAA,
    MediumBlue = 0x0000CD,
    MediumOrchid = 0xBA55D3,
    MediumPurple = 0x9370DB,
    MediumSeaGreen = 0x3CB371,
    MediumSlateBlue = 0x7B68EE,
    MediumSpringGreen = 0x00FA9A,
    MediumTurquoise = 0x48D1CC,
    MediumVioletRed = 0xC71585,
    MidnightBlue = 0x191970,
    MintCream = 0xF5FFFA,
    MistyRose = 0xFFE4E1,
    Moccasin = 0xFFE4B5,
    NavajoWhite = 0xFFDEAD,
    Navy = 0x000080,
    OldLace = 0xFDF5E6,
    Olive = 0x808000,
    OliveDrab = 0x6B8E23,
    Orange = 0xFFA500,
    OrangeRed = 0xFF4500,
    Orchid = 0xDA70D6,
    PaleGoldenrod = 0xEEE8AA,
    PaleGreen = 0x98FB98,
    PaleTurquoise = 0xAFEEEE,
    PaleVioletRed = 0xDB7093,
    PapayaWhip = 0xFFEFD5,
    PeachPuff = 0xFFDAB9,
    Peru = 0xCD853F,
    Pink = 0xFFC0CB,
    Plum = 0xDDA0DD,
    PowderBlue = 0xB0E0E6,
    Purple = 0x800080,
    Red = 0xFF0000,
    RosyBrown = 0xBC8F8F,
    RoyalBlue = 0x4169E1,
    SaddleBrown = 0x8B4513,
    Salmon = 0xFA8072,
    SandyBrown = 0xF4A460,
    SeaGreen = 0x2E8B57,
    SeaShell = 0xFFF5EE,
    Sienna = 0xA0522D,
    Silver = 0xC0C0C0,
    SkyBlue = 0x87CEEB,
    SlateBlue = 0x6A5ACD,
    SlateGray = 0x708090,
    Snow = 0xFFFAFA,
    SpringGreen = 0x00FF7F,
    SteelBlue = 0x4682B4,
    Tan = 0xD2B48C,
    Teal = 0x008080,
    Thistle = 0xD8BFD8,
    Tomato = 0xFF6347,
    Turquoise = 0x40E0D0,
    Violet = 0xEE82EE,
    Wheat = 0xF5DEB3,
    White = 0xFFFFFF,
    WhiteSmoke = 0xF5F5F5,
    Yellow = 0xFFFF00,
    YellowGreen = 0x9ACD32,
};
enum class TextFontStretch {   
    UNDEFINED = 0,
    ULTRA_CONDENSED = 1,
    EXTRA_CONDENSED = 2,
    CONDENSED = 3,
    SEMI_CONDENSED = 4,
    NORMAL = 5,
    MEDIUM = 5,
    SEMI_EXPANDED = 6,
    EXPANDED = 7,
    EXTRA_EXPANDED = 8,
    ULTRA_EXPANDED = 9
};

struct TextOption {
	std::string	    Font = "Verdana";
	float			FontSize = 20.f;
    TextColor		FontColor = TextColor::White;
    TextFontWeight	FontWeight = TextFontWeight::NORMAL;
    TextFontStyle	FontStyle = TextFontStyle::Normal;
    TextFontStretch	FontStretch = TextFontStretch::NORMAL;
    TextAlignType	HAlignment = TextAlignType::Center;
    TextAlignType	VAlignment = TextAlignType::Center;
	Vec2			BoxExtent = Vec2::Zero;
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
	void SetColor(TextColor color);
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
	Vec2 GetTextCenter() const;
};

class TextMgr : public Singleton<TextMgr> {
	friend Singleton;

private:
	std::set<sptr<TextBox>> mTextBoxes{};

public:
	void AddTextBox(rsptr<TextBox> textBox) { mTextBoxes.insert(textBox); }
	void RemoveTextBox(rsptr<TextBox> textBox) { mTextBoxes.erase(textBox); }
	//void CreateTextBox(const std::string& text, float fontSize, TextAlignType align = TextAlignType::Center, TextWeightType weight = TextWeightType::NORMAL, FontStyle style);

	void Render(RComPtr<struct ID2D1DeviceContext2> device);
	void Reset();
	void CreateBrush();
};

