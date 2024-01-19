#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region Define
#define canvas Canvas::Inst()
#pragma endregion


#pragma region ClassForwardDecl
class CanvasShader;
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

	static sptr<ModelObjectMesh> mMesh;		// 2D plane mesh

public:
	UI() : Transform(this) {}
	virtual ~UI() = default;

public:
	// [texture]�� �����ϰ�, [pos]��ġ�� [width * height] ũ���� UI�� �����Ѵ�.
	void Create(rsptr<Texture> texture, Vec3 pos, float width, float height);

	void Update();
	void UpdateShaderVars() const;

	virtual void Render();

	// create [mMesh]
	static void CreateUIMesh();
	// delete [mMesh]
	static void DeleteUIMesh();
};



// for render font
// comdef.h�� Font�� �ߺ��Ǿ� �̸� ���� Font -> MyFont
class MyFont : public UI {
private:
	std::string mText{};	// "YOUR SCORE IS "
	std::string mScore{};

	mutable std::vector<int> mObjCBIdxes{};

public:
	MyFont() = default;
	virtual ~MyFont() = default;

	void SetText(const std::string& text) { mText = text; }
	void SetScore(const std::string& score) { mScore = score; }

public:
	// [pos]��ġ�� [width * height] ũ���� UI�� �����Ѵ�.
	void Create(const Vec3& pos, float width, float height);

	// �ϳ��� �̹������� Ư�� ���ڸ� �����ϴ� matrix�� ������ set(SetGraphicsRoot32BitConstants)�Ѵ�. (x)
	// ��� ���� �信 set �ϱ� ���ؼ� ����� �����Ͽ���.
	// mObjCBIdxes�� �ִ� �ε����� �����Ͽ� set(SetGraphicsRootConstantBufferView)�Ѵ�.
	void UpdateShaderVars(char ch, int cnt) const;

	virtual void OnDisable() override;
	virtual void Render() override;

	void CreateFontTexture();
	void ReleaseFontTexture();
};





// Canvas ���� UI�� �׸����� �Ѵ�.
class Canvas : public Singleton<Canvas> {
	friend class Singleton;

private:
	std::vector<sptr<UI>>	mUIs{};		// all UIs
	sptr<MyFont>			mFont{};
	sptr<CanvasShader>		mShader{};

	std::unordered_map<std::string, sptr<Texture>> mTextureMap{}; // UI folder���� load�� ��� UI texture ����

private:
	Canvas() = default;
	virtual ~Canvas() = default;

public:
	rsptr<Texture> GetTexture(const std::string& name) const { return mTextureMap.at(name); }

	void SetScore(int score);

public:
	void Init();
	void Release();
	void BuildUIs();

	void Update();
	void Render() const;

private:
	// UI folder�� ��� UI texutre���� �ε��Ѵ�.
	void LoadTextures();
};
#pragma endregion
