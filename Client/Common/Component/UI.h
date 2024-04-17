#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Canvas;
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
	// [texture]�� �����ϰ�, [pos]��ġ�� [width * height] ũ���� UI�� �����Ѵ�.
	UI(rsptr<Texture> texture, Vec2 pos, float width, float height);
	virtual ~UI() = default;

public:
	virtual void Update() {}
	virtual void Render();

	void SetPosition(float x, float y, float z);
	void SetPosition(const Vec2& pos);
	void SetPosition(const Vec3& pos);

private:
	void UpdateShaderVars() const;
};



// for render font
// comdef.h�� Font�� �ߺ��Ǿ� �̸� ���� Font -> MyFont
class MyFont : public UI {
private:
	std::string mText{};	// "YOUR SCORE IS "
	std::string mScore{};

public:
	// [pos]��ġ�� [width * height] ũ���� UI�� �����Ѵ�.
	MyFont(const Vec2& pos, float width, float height);
	virtual ~MyFont() = default;

	void SetText(const std::string& text) { mText = text; }
	void SetScore(const std::string& score) { mScore = score; }

public:
	virtual void Render() override;

private:
	// �ϳ��� �̹������� Ư�� ���ڸ� �����ϴ� matrix�� ������ set(SetGraphicsRoot32BitConstants)�Ѵ�. (x)
	// ��� ���� �信 set �ϱ� ���ؼ� ����� �����Ͽ���.
	// mObjCBIdxes�� �ִ� �ε����� �����Ͽ� set(SetGraphicsRootConstantBufferView)�Ѵ�.
	void UpdateShaderVars(char ch, int cnt) const;
};





// Canvas ���� UI�� �׸����� �Ѵ�.
class Canvas : public Singleton<Canvas> {
	friend Singleton;

private:
	std::unordered_set<sptr<UI>>	mUIs{};		// all UIs
	sptr<MyFont>					mFont{};

	float mWidth{};
	float mHeight{};

private:
	Canvas() = default;
	virtual ~Canvas() = default;

public:
	void SetScore(int score);

	float GetWidth() const { return mWidth; }
	float GetHeight() const { return mHeight; }

public:
	void Init();
	void BuildUIs();

	void Update();
	void Render() const;
	sptr<UI> CreateUI(const std::string& texture, const Vec2& pos, float width, float height);
};
#pragma endregion
