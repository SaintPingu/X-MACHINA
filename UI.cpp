#include "stdafx.h"
#include "UI.h"
#include "DXGIMgr.h"

#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "FileIO.h"



#pragma region UI
sptr<ModelObjectMesh> UI::mMesh;

void UI::Create(rsptr<Texture> texture, Vec3 pos, float width, float height)
{
	mTexture = texture;

	pos.x /= gkFrameBufferWidth;
	pos.y /= gkFrameBufferHeight;

	mWidth = width / gkFrameBufferWidth;
	mHeight = height / gkFrameBufferHeight;

	SetPosition(pos);
}

void UI::Update()
{

}

void UI::UpdateShaderVars() const
{
	Matrix scaling = XMMatrixMultiply(XMMatrixScaling(mWidth, mHeight, 1.f), _MATRIX(GetWorldTransform()));
	scene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrixTranspose(scaling), 0);
}

void UI::Render()
{
	mTexture->UpdateShaderVars();
	UpdateShaderVars();

	mMesh->Render();
}

void UI::CreateUIMesh()
{
	mMesh = std::make_shared<ModelObjectMesh>();
	mMesh->CreatePlaneMesh(1, 1, false);
}

void UI::DeleteUIMesh()
{
	mMesh = nullptr;
}
#pragma endregion





#pragma region Font
sptr<Texture> Font::mFontTexture;

void Font::Create(const Vec3& pos, float width, float height)
{
	UI::Create(nullptr, pos, width, height);
}

void Font::UpdateShaderVarSprite(char ch) const
{
	constexpr float cols = 8.f;
	constexpr float rows = 5.f;

	if (isalpha(ch)) {
		ch -= 'A';
	}
	else if (isdigit(ch)) {
		ch -= '0' - 26;
	}
	float col = ch % int(cols);
	float row = int(ch / cols);

	Vec4x4 spriteMtx = Matrix4x4::Identity();

	spriteMtx._11 = 1.f / cols;
	spriteMtx._22 = 1.f / rows;
	spriteMtx._31 = col / cols;
	spriteMtx._32 = row / rows;

	scene->SetGraphicsRoot32BitConstants(RootParam::SpriteInfo, XMMatrix::Transpose(spriteMtx), 0);
}

void Font::Render()
{
	mFontTexture->UpdateShaderVars();

	Vec3 originPos = GetPosition();
	Vec3 fontPos = GetPosition();

	for (char ch : mText) {
		UpdateShaderVars();
		if (ch != ' ') {
			UpdateShaderVarSprite(ch);
			mMesh->Render();
		}

		fontPos.x += 0.07f;
		SetPosition(fontPos);
	}
	for (char ch : mScore) {
		UpdateShaderVars();
		if (ch != ' ') {
			UpdateShaderVarSprite(ch);
			mMesh->Render();
		}

		fontPos.x += 0.07f;
		SetPosition(fontPos);
	}

	SetPosition(originPos);
}

void Font::SetFontTexture()
{
	mFontTexture = canvas->GetTexture("Alphabet");
}

void Font::UnSetFontTexture()
{
	mFontTexture = nullptr;
}
#pragma endregion








#pragma region Canvas
SINGLETON_PATTERN_DEFINITION(Canvas)

void Canvas::SetScore(int score)
{
	mFont->SetScore(std::to_string(score));
}

void Canvas::Init()
{
	mShader = std::make_shared<CanvasShader>();
	mShader->Create();
	LoadTextures();

	UI::CreateUIMesh();
	Font::SetFontTexture();

	BuildUIs();
}

void Canvas::Release()
{
	UI::DeleteUIMesh();
	Font::UnSetFontTexture();

	Destroy();
}

void Canvas::BuildUIs()
{
	mFont = std::make_shared<Font>();
	mFont->Create(Vec3(-600, 900, 0), 100, 100);
	mFont->SetText("YOUR SCORE IS ");
	mFont->SetScore("0");
}

void Canvas::Update()
{
	for (auto& ui : mUIs) {
		ui->Update();
	}
}

void Canvas::Render() const
{
	mShader->Render();
	for (auto& ui : mUIs) {
		ui->Render();
	}
	mFont->Render();
}

void Canvas::LoadTextures()
{
	std::vector<std::string> textureNames{};
	FileIO::GetTextureNames(textureNames, "Models/UI");

	// load textures
	for (auto& textureName : textureNames) {
		sptr<Texture> texture = std::make_shared<Texture>(Resource::Texture2D);
		texture->LoadUITexture(textureName);

		mTextureMap.insert(std::make_pair(textureName, texture));
	}
}
#pragma endregion
