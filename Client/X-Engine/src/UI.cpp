#include "EnginePch.h"
#include "Component/UI.h"

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
UI::UI(rsptr<Texture> texture, Vec2 pos, float width, float height)
	:
	Transform(this),
	mTexture(texture)
{
	short windowWidth = dxgi->GetWindowWidth();
	short windowHeight = dxgi->GetWindowHeight();

	pos.x /= windowWidth;
	pos.y /= windowHeight;

	mWidth = width / windowWidth;
	mHeight = height / windowHeight;

	// 오브젝트 상수 버퍼 사용 플래그는 직접 설정할 수 있다.
	SetUseObjCB(true);
	SetPosition(pos);
}

void UI::UpdateShaderVars() const
{
	Transform::UpdateShaderVars();

	ObjectConstants objectConstants;
	objectConstants.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mWidth, mHeight, 1.f), _MATRIX(GetWorldTransform())));
	objectConstants.MatIndex = mTexture->GetSrvIdx();

	frmResMgr->CopyData(mObjCBIndices.front(), objectConstants);
	dxgi->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(mObjCBIndices.front()));
}


void UI::Render()
{
	if (!mTexture) {
		return;
	}

	UpdateShaderVars();

	res->Get<ModelObjectMesh>("Rect")->Render();
}

#pragma endregion





#pragma region Font
MyFont::MyFont(const Vec2& pos, float width, float height)
	:
	UI(res->Get<Texture>("Alphabet"), pos, width, height)
{
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

	Matrix spriteMtx{};

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
	objectConstants.MtxSprite	= spriteMtx.Transpose();
	objectConstants.MatIndex	= mTexture->GetSrvIdx(); 

	frmResMgr->CopyData(mObjCBIndices[cnt], objectConstants);
	dxgi->SetGraphicsRootConstantBufferView(RootParam::Object, frmResMgr->GetObjCBGpuAddr(mObjCBIndices[cnt]));
}

void MyFont::Render()
{
	if (!mTexture) {
		return;
	}

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
	Destroy();
}

void Canvas::BuildUIs()
{
	mFont = std::make_shared<MyFont>(Vec2(-600, 900), 100.f, 100.f);
	//mFont->SetText("YOUR SCORE IS ");
	//mFont->SetScore("0");
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

	//mFont->Render();
}

sptr<UI> Canvas::CreateUI(const std::string& texture, const Vec2& pos, float width, float height)
{
	sptr<UI> ui = std::make_shared<UI>(res->Get<Texture>(texture), pos, width, height);
	mUIs.insert(ui);
	return ui;
}
#pragma endregion
