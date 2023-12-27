#pragma once
#include "Component.h"

class CCanvasShader;
class CTexture;
class CModelObjectMesh;

class UI : public Transform {
protected:
	static sptr<CModelObjectMesh> mesh;

public:
	static void CreateUIMesh();
	static void DeleteUIMesh() { mesh = nullptr; }

protected:
	sptr<CTexture> mTexture{};
	float mWidth{};
	float mHeight{};

public:
	void Create(rsptr<CTexture> texture, Vec3 pos, float width, float height);

	void Update();
	void UpdateShaderVariable() const;

	virtual void Render();
};

class Font : public UI {
protected:
	static sptr<CTexture> fontTexture;

public:
	static void SetFontTexture();
	static void UnSetFontTexture() { fontTexture = nullptr; }

private:
	std::string mText{};
	std::string mScore{};

public:
	void Create(const Vec3& pos, float width, float height);

	void SetText(const std::string& text) { mText = text; }
	void SetScore(const std::string& score) { mScore = score; }
	void UpdateShaderVariableSprite(char ch) const;

	virtual void Render() override;

};

class Canvas {
	SINGLETON_PATTERN(Canvas)
private:
	vector<sptr<UI>> mUIs{};
	sptr<Font> mFont{};
	sptr<CCanvasShader> mShader{};
	std::unordered_map<std::string, sptr<CTexture>> mTextureMap{};

	void LoadTextures();
public:
	rsptr<CTexture> GetTexture(const std::string& name) const { return mTextureMap.at(name); }

	void Create();
	void OnDestroy();
	void BuildUIs();

	void Update();
	void Render() const;

	void SetScore(int score);
};