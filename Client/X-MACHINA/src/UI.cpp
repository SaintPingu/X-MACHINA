#include "stdafx.h"
#include "UI.h"

#include "Scene.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "FileIO.h"

#include "DXGIMgr.h"
#include "FrameResource.h"


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
	ObjectConstants objectConstants;
	objectConstants.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mWidth, mHeight, 1.f), _MATRIX(GetWorldTransform())));
	frmResMgr->CopyData(mObjCBIdx, objectConstants);

	scene->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(mObjCBIdx));
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
void MyFont::Create(const Vec3& pos, float width, float height)
{
	UI::Create(nullptr, pos, width, height);
	mObjCBIdxes = std::vector<int>(30, -1);
}

void MyFont::UpdateShaderVars(char ch, int cnt) const
{
	constexpr float kCols = 8.f;
	constexpr float kRows = 5.f;

	if (isalpha(ch)) {
		ch -= 'A';
	}
	else if (isdigit(ch)) {
		ch -= '0' - 26;
	}
	const float col = (float)(ch % int(kCols));
	const float row = (float)(int(ch / kCols));

	Vec4x4 spriteMtx = Matrix4x4::Identity();

	spriteMtx._11 = 1.f / kCols;
	spriteMtx._22 = 1.f / kRows;
	spriteMtx._31 = col / kCols;
	spriteMtx._32 = row / kRows;
	
	// 객체를 렌더링하지 않는다면 굳이 루트 상수를 set할 필요가 없기 때문에 렌더링을 해야 하는 경우에만 set하도록 변경
	ObjectConstants objectConstants;
	objectConstants.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mWidth, mHeight, 1.f), _MATRIX(GetWorldTransform())));
	objectConstants.MtxSprite = XMMatrix::Transpose(spriteMtx);

	// -1로 초기화 되어 있는 mObjCBIdxes에 가용 인덱스를 저장하고 해당 배열에 있는 인덱스로만 set을 한다.
	frmResMgr->CopyData(mObjCBIdxes[cnt], objectConstants);
	scene->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(mObjCBIdxes[cnt]));
}

void MyFont::OnDisable()
{
	for (const auto objCBIdx : mObjCBIdxes) {
		frmResMgr->ReturnObjCBIdx(mObjCBIdx);
	}
}

void MyFont::Render()
{
	mTexture->UpdateShaderVars();

	Vec3 originPos = GetPosition();
	Vec3 fontPos = GetPosition();

	int cnt{};
	for (char ch : mText) {
		// 렌더링하지 않는다면 굳이 루트 상수를 set할 필요가 없다.
		if (ch != ' ') {
			UpdateShaderVars(ch, cnt++);
			mMesh->Render();
		}

		fontPos.x += 0.07f;
		SetPosition(fontPos);
	}
	for (char ch : mScore) {
		if (ch != ' ') {
			UpdateShaderVars(ch, cnt++);
			mMesh->Render();
		}

		fontPos.x += 0.07f;
		SetPosition(fontPos);
	}

	SetPosition(originPos);
}

void MyFont::CreateFontTexture()
{
	mTexture = canvas->GetTexture("Alphabet");
}

void MyFont::ReleaseFontTexture()
{
	mTexture = nullptr;
}
#pragma endregion








#pragma region Canvas
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

	BuildUIs();
}

void Canvas::Release()
{
	UI::DeleteUIMesh();
	mFont->ReleaseFontTexture();
	mFont->OnDisable();
	Destroy();
}

void Canvas::BuildUIs()
{
	mFont = std::make_shared<MyFont>();
	mFont->Create(Vec3(-600, 900, 0), 100, 100);
	mFont->CreateFontTexture();
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
	mShader->Set();
	for (auto& ui : mUIs) {
		ui->Render();
	}
	mFont->Render();
}

void Canvas::LoadTextures()
{
	mTextureMap = FileIO::LoadTextures("Models/UI/");
}
#pragma endregion
