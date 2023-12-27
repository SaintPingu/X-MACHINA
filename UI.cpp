#include "stdafx.h"
#include "UI.h"
#include "DXGIMgr.h"

#include "Camera.h"
#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"

SINGLETON_PATTERN_DEFINITION(Canvas)

sptr<ModelObjectMesh> UI::mesh;
sptr<Texture> Font::fontTexture;

void UI::CreateUIMesh()
{
	mesh = std::make_shared<ModelObjectMesh>(1, 1, false);
}

void UI::Create(rsptr<Texture> texture, Vec3 pos, float width, float height)
{
	pos.x /= FRAME_BUFFER_WIDTH;
	pos.y /= FRAME_BUFFER_HEIGHT;
	mWidth = width / FRAME_BUFFER_WIDTH;
	mHeight = height / FRAME_BUFFER_HEIGHT;

	mTexture = texture;
	SetPosition(pos);
}

void UI::Update()
{

}

void UI::UpdateShaderVariable() const
{
	XMMATRIX scaling = XMMatrixMultiply(XMMatrixScaling(mWidth, mHeight, 1.f), XMLoadFloat4x4(&GetWorldTransform()));

	crntScene->SetGraphicsRoot32BitConstants(RootParam::GameObjectInfo, XMMatrixTranspose(scaling), 0);
}

void UI::Render()
{
	mTexture->UpdateShaderVariables();
	UpdateShaderVariable();

	mesh->Render();
}



void Font::SetFontTexture()
{
	Font::fontTexture = Canvas::Inst()->GetTexture("Alphabet");
}

void Font::Create(const Vec3& pos, float width, float height)
{
	UI::Create(nullptr, pos, width, height);
}

void Font::UpdateShaderVariableSprite(char ch) const
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

	spriteMtx._11 = 1.0f / cols;
	spriteMtx._22 = 1.0f / rows;
	spriteMtx._31 = col / cols;
	spriteMtx._32 = row / rows;

	crntScene->SetGraphicsRoot32BitConstants(RootParam::SpriteInfo, XMMatrix::Transpose(spriteMtx), 0);
}

void Font::Render()
{
	fontTexture->UpdateShaderVariables();

	Vec3 originPos = GetPosition();
	Vec3 fontPos = GetPosition();

	for (char ch : mText) {
		UpdateShaderVariable();
		if (ch != ' ') {
			UpdateShaderVariableSprite(ch);
			mesh->Render();
		}

		fontPos.x += 0.07f;
		SetPosition(fontPos);
	}
	for (char ch : mScore) {
		UpdateShaderVariable();
		if (ch != ' ') {
			UpdateShaderVariableSprite(ch);
			mesh->Render();
		}

		fontPos.x += 0.07f;
		SetPosition(fontPos);
	}

	SetPosition(originPos);
}







void Canvas::LoadTextures()
{
	std::vector<std::string> textureNames{};
	::LoadTextureNames(textureNames, "Models/UI");

	for (auto& textureName : textureNames) {
		// load texture
		sptr<Texture> texture = std::make_shared<Texture>(RESOURCE_TEXTURE2D);
		texture->LoadUITexture(textureName);

		mTextureMap.insert(std::make_pair(textureName, texture));
	}
}


void Canvas::Create()
{
	mShader = std::make_shared<CanvasShader>();
	mShader->CreateShader();
	LoadTextures();

	UI::CreateUIMesh();
	Font::SetFontTexture();

	BuildUIs();
}

void Canvas::OnDestroy()
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


void Canvas::SetScore(int score)
{
	mFont->SetScore(std::to_string(score));
}