#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region Define
#define canvas Canvas::Inst()
#pragma endregion


#pragma region ClassForwardDecl
class Texture;
class ModelObjectMesh;
#pragma endregion


#pragma region Class
// 2D object (not entity)
class UI : public Transform {
protected:
	sptr<Texture>	mTexture{};		// image
	float			mWidth{};
	float			mHeight{};

public:
	UI() : Transform(this) {}
	virtual ~UI() = default;

public:
	// [texture]를 설정하고, [pos]위치에 [width * height] 크기의 UI를 생성한다.
	void Create(rsptr<Texture> texture, Vec3 pos, float width, float height);

	void Update();
	void UpdateShaderVars() const;

	virtual void Render();
};



// for render font
// comdef.h의 Font와 중복되어 이름 변경 Font -> MyFont
class MyFont : public UI {
private:
	std::string mText{};	// "YOUR SCORE IS "
	std::string mScore{};

public:
	MyFont() = default;
	virtual ~MyFont() = default;

	void SetText(const std::string& text) { mText = text; }
	void SetScore(const std::string& score) { mScore = score; }

public:
	// [pos]위치에 [width * height] 크기의 UI를 생성한다.
	void Create(const Vec3& pos, float width, float height);

	// 하나의 이미지에서 특정 문자를 지정하는 matrix를 추출해 set(SetGraphicsRoot32BitConstants)한다. (x)
	// 상수 버퍼 뷰에 set 하기 위해서 방식을 변경하였다.
	// mObjCBIdxes에 있는 인덱스를 참고하여 set(SetGraphicsRootConstantBufferView)한다.
	void UpdateShaderVars(char ch, int cnt) const;

	virtual void Render() override;

	void CreateFontTexture();
	void ReleaseFontTexture();
};





// Canvas 위에 UI를 그리도록 한다.
class Canvas : public Singleton<Canvas> {
	friend Singleton;

private:
	std::vector<sptr<UI>>	mUIs{};		// all UIs
	sptr<MyFont>			mFont{};

private:
	Canvas() = default;
	virtual ~Canvas() = default;

public:
	void SetScore(int score);

public:
	void Init();
	void Release();
	void BuildUIs();

	void Update();
	void Render() const;
};
#pragma endregion
