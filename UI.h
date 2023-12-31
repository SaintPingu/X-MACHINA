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
class UI : public Transform {
protected:
	sptr<Texture>	mTexture{};
	float			mWidth{};
	float			mHeight{};

	static sptr<ModelObjectMesh> mMesh;

public:
	UI() = default;
	virtual ~UI() = default;

public:
	void Create(rsptr<Texture> texture, Vec3 pos, float width, float height);

	void Update();
	void UpdateShaderVars() const;

	virtual void Render();

	static void CreateUIMesh();
	static void DeleteUIMesh();
};





class Font : public UI {
protected:
	static sptr<Texture> mFontTexture;

private:
	std::string mText{};
	std::string mScore{};

public:
	Font() = default;
	virtual ~Font() = default;

	void SetText(const std::string& text) { mText = text; }
	void SetScore(const std::string& score) { mScore = score; }

public:
	void Create(const Vec3& pos, float width, float height);

	void UpdateShaderVarSprite(char ch) const;

	virtual void Render() override;

	static void SetFontTexture();
	static void UnSetFontTexture();
};





class Canvas {
	SINGLETON_PATTERN(Canvas)

private:
	std::vector<sptr<UI>>	mUIs{};
	sptr<Font>				mFont{};
	sptr<CanvasShader>		mShader{};

	std::unordered_map<std::string, sptr<Texture>> mTextureMap{};

public:
	Canvas() = default;
	virtual ~Canvas() = default;

	rsptr<Texture> GetTexture(const std::string& name) const { return mTextureMap.at(name); }

	void SetScore(int score);

public:
	void Init();
	void Release();
	void BuildUIs();

	void Update();
	void Render() const;

private:
	void LoadTextures();
};
#pragma endregion
