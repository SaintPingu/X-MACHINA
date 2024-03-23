#include "stdafx.h"
#include "UI.h"

#include "Scene.h"
#include "ResourceMgr.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "FileIO.h"

#include "DXGIMgr.h"
#include "FrameResource.h"
#include "MultipleRenderTarget.h"


#pragma region UI
void UI::Create(rsptr<Texture> texture, Vec3 pos, float width, float height)
{
	mTexture = texture;

	short windowWidth = dxgi->GetWindowWidth();
	short windowHeight = dxgi->GetWindowWidth();

	pos.x /= windowWidth;
	pos.y /= windowHeight;

	mWidth = width / windowWidth;
	mHeight = height / windowHeight;

	// 오브젝트 상수 버퍼 사용 플래그는 직접 설정할 수 있다.
	SetUseObjCB(true);
	SetPosition(pos);
}

void UI::Update()
{

}

void UI::UpdateShaderVars() const
{
	ObjectConstants objectConstants;
	objectConstants.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mWidth, mHeight, 1.f), _MATRIX(GetWorldTransform())));
	frmResMgr->CopyData(mObjCBIndices.front(), objectConstants);

	dxgi->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(mObjCBIndices.front()));
}

void UI::Render()
{
	UpdateShaderVars();

	res->Get<ModelObjectMesh>("Rect")->Render();
}

#pragma endregion





#pragma region Font
void MyFont::Create(const Vec3& pos, float width, float height)
{
	UI::Create(nullptr, pos, width, height);
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
	
	if (mObjCBCount <= cnt) {
		mObjCBCount = cnt + 1;
	}

	// 머티리얼을 사용하지 않는 경우 MatIndex에 텍스처 인덱스를 넣어준다.
	ObjectConstants objectConstants;
	objectConstants.MtxWorld	= XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mWidth, mHeight, 1.f), _MATRIX(GetWorldTransform())));
	objectConstants.MtxSprite	= XMMatrix::Transpose(spriteMtx);
	objectConstants.MatIndex	= mTexture->GetSrvIdx(); 

	frmResMgr->CopyData(mObjCBIndices[cnt], objectConstants);
	dxgi->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(mObjCBIndices[cnt]));
}

void MyFont::Render()
{
	Vec3 originPos = GetPosition();
	Vec3 fontPos = GetPosition();

	int cnt{ 0 };
	for (char ch : mText) {
		// 렌더링하지 않는다면 굳이 루트 상수를 set할 필요가 없다.
		if (ch != ' ') {
			UpdateShaderVars(ch, cnt++);
			res->Get<ModelObjectMesh>("Rect")->Render();
		}

		fontPos.x += 0.07f;
		SetPosition(fontPos);
	}
	for (char ch : mScore) {
		if (ch != ' ') {
			UpdateShaderVars(ch, cnt++);
			res->Get<ModelObjectMesh>("Rect")->Render();
		}

		fontPos.x += 0.07f;
		SetPosition(fontPos);
	}

	SetPosition(originPos);
}

void MyFont::CreateFontTexture()
{
	mTexture = res->Get<Texture>("Alphabet");
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
	BuildUIs();
}

void Canvas::Release()
{
	mFont->ReleaseFontTexture();
	mFont->OnDestroy();
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
	res->Get<Shader>("Canvas")->Set();
	for (auto& ui : mUIs) {
		ui->Render();
	}
	mFont->Render();
}

#pragma endregion
