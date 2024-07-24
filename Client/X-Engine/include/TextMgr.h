#pragma once

class DXGIMgr;

class TextFontColor {
private:
    static constexpr UINT32 sc_redShift = 16;
    static constexpr UINT32 sc_greenShift = 8;
    static constexpr UINT32 sc_blueShift = 0;

    static constexpr UINT32 sc_redMask = 0xff << sc_redShift;
    static constexpr UINT32 sc_greenMask = 0xff << sc_greenShift;
    static constexpr UINT32 sc_blueMask = 0xff << sc_blueShift;

public:
    float r{};
    float g{};
    float b{};
    float a{};

	enum class Type {
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

    TextFontColor(const TextFontColor::Type type) { Init(type); }

    void Init(const TextFontColor::Type type)
    {
        UINT32 rgb = static_cast<UINT32>(type);
        r = static_cast<float>((rgb & sc_redMask) >> sc_redShift) / 255.f;
        g = static_cast<float>((rgb & sc_greenMask) >> sc_greenShift) / 255.f;
        b = static_cast<float>((rgb & sc_blueMask) >> sc_blueShift) / 255.f;
        a = 1.0f;
    }
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
enum class TextAlignType {
    Leading, Trailing, Center, Justified
};
enum class TextParagraphAlign {
	Near, Far, Center
};

struct TextOption {
	std::string	        Font        = "Verdana";
	float			    FontSize    = 20.f;
    TextFontColor		FontColor   = TextFontColor::Type::White;
    TextFontWeight	    FontWeight  = TextFontWeight::NORMAL;
    TextFontStyle	    FontStyle   = TextFontStyle::Normal;
    TextFontStretch	    FontStretch = TextFontStretch::NORMAL;
    TextAlignType	    HAlignment  = TextAlignType::Center;
    TextParagraphAlign	VAlignment  = TextParagraphAlign::Center;
	Vec2			    BoxExtent   = Vec2::Zero;
};

class TextMatrix {
public:
    union
    {
        struct
        {
            float _11, _12;
            float _21, _22;
            float _31, _32;
        };
        float m[3][2];
    };

public:
    TextMatrix();
    
public:
    inline void SetProduct(const TextMatrix& a, const TextMatrix& b)
    {
        _11 = a._11 * b._11 + a._12 * b._21;
        _12 = a._11 * b._12 + a._12 * b._22;
        _21 = a._21 * b._11 + a._22 * b._21;
        _22 = a._21 * b._12 + a._22 * b._22;
        _31 = a._31 * b._11 + a._32 * b._21 + b._31;
        _32 = a._31 * b._12 + a._32 * b._22 + b._32;
    }

public:
    static TextMatrix Identity();
};


class TextBox {
private:
	TextOption mOption{};

	ComPtr<struct ID2D1SolidColorBrush> mTextBrush{};
	ComPtr<struct IDWriteTextFormat> mTextFormat{};

	std::wstring mText{};

    TextMatrix mMtxFinal{};
    TextMatrix mMtxScale{};
    TextMatrix mMtxRotation{};
    TextMatrix mMtxTransition{};

public:
    TextBox(const TextOption& option);
	void CreateBrush();

public:
	Vec2 GetTextCenter() const;
	Vec2 GetPosition() const;
    float GetAlpha() const;

public:
	void SetPosition(const Vec2& pos);
	void SetRotation(float angle);
	void SetScale(const Vec2& scale);
	void SetColor(TextFontColor color);
	void SetAlpha(float alpha);

public:	
	void SetText(const std::string& text);
	void SetText(const std::wstring& text);
	void AddAlpha(float alpha);

public:
	void Render(RComPtr<struct ID2D1DeviceContext2> device) const;

private:
	void SetFinalMatrix();
};

class TextMgr : public Singleton<TextMgr> {
	friend Singleton;
	friend DXGIMgr;

private:
	std::unordered_set<sptr<TextBox>> mTextBoxes{};

public:
    TextMgr() = default;
    virtual ~TextMgr() = default;

public:
	TextBox* CreateText(const std::string& text = "text", const Vec2& pos = Vec2::Zero, const TextOption & option = TextOption{});
    void RemoveTextBox(TextBox* textBox);

	void Render(RComPtr<struct ID2D1DeviceContext2> device);
	void Clear();

private:
	void CreateBrush();
};

