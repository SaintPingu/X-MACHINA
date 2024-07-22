#include "EnginePch.h"
#include "Component/UI.h"

#include "BattleScene.h"
#include "ResourceMgr.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "FileIO.h"

#include "DXGIMgr.h"
#include "FrameResource.h"
#include "MultipleRenderTarget.h"


#pragma region UITexture
UITexture::UITexture(const std::string& imageName, float width, float height)
{
	UIImage = RESOURCE<Texture>(imageName);
	Width = width / Canvas::I->GetWidth();
	Height = height / Canvas::I->GetHeight();
}
#pragma endregion


#pragma region UI
UI::UI(const std::string& textureName, Vec2 pos, float width, float height, rsptr<Shader> shader, const std::string& name)
	:
	Transform(this),
	mShader(shader)
{
	mUITexture = std::make_shared<UITexture>(textureName, width, height);
	mObjectCB.SliderValue = 1.f;

	// ������Ʈ ��� ���� ��� �÷��״� ���� ������ �� �ִ�.
	SetUseObjCB(true);
	SetPosition(pos);

	mName = name;
	if (mName == "") {
		mName = textureName;
	}
}

void UI::ChangeUITexture(rsptr<UITexture> newUITexture)
{
	mUITexture = newUITexture;
}
bool UI::CheckInteract(const Vec2& mousePos)
{
	Vec2 pos = GetPosition();
	float width = mUITexture->Width;
	float height = mUITexture->Height;

	float left = pos.x - (width / 2);
	float right = pos.x + (width / 2);
	float up = pos.y + (height / 2);
	float down = pos.y - (height / 2);

	if (pos.x < left) {
		return false;
	}
	if (pos.x > right) {
		return false;
	}
	if (pos.y < down) {
		return false;
	}
	if (pos.y > up) {
		return false;
	}

	return true;
}

void UI::Interact()
{

}


void UI::UpdateShaderVars()
{
	mObjectCB.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mUITexture->Width, mUITexture->Height, 1.f), _MATRIX(GetWorldTransform())));
	mObjectCB.MatIndex = mUITexture->UIImage->GetSrvIdx();

	FRAME_RESOURCE_MGR->CopyData(mObjCBIndices.front(), mObjectCB);
	DXGIMgr::I->SetGraphicsRootConstantBufferView(RootParam::Object, FRAME_RESOURCE_MGR->GetObjCBGpuAddr(mObjCBIndices.front()));
}

void UI::Render()
{
	if (!mIsActive) {
		return;
	}

	if (!mUITexture->UIImage) {
		return;
	}

	if (mShader) {
		mShader->Set();
	}
	else {
		RESOURCE<Shader>("Canvas")->Set();
	}

	UpdateShaderVars();

	RESOURCE<ModelObjectMesh>("Rect")->Render();
}

void UI::SetPosition(float x, float y, float z)
{
	x /= Canvas::I->GetWidth();
	y /= Canvas::I->GetHeight();
	Transform::SetPosition(x, y, z);
}

void UI::SetPosition(const Vec2& pos)
{
	SetPosition(pos.x, pos.y, 0.f);
}

void UI::SetPosition(const Vec3& pos)
{
	SetPosition(pos.x, pos.y, pos.z);
}

#pragma endregion





#pragma region Font
MyFont::MyFont(const Vec2& pos, float width, float height)
	:
	UI("Alphabet", pos, width, height)
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

	// ��Ƽ������ ������� �ʴ� ��� MatIndex�� �ؽ�ó �ε����� �־��ش�.
	ObjectConstants objectConstants;
	objectConstants.MtxWorld	= XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mUITexture->Width, mUITexture->Height, 1.f), _MATRIX(GetWorldTransform())));
	objectConstants.MtxSprite	= spriteMtx.Transpose();
	objectConstants.MatIndex	= mUITexture->UIImage->GetSrvIdx();

	FRAME_RESOURCE_MGR->CopyData(mObjCBIndices[cnt], objectConstants);
	DXGIMgr::I->SetGraphicsRootConstantBufferView(RootParam::Object, FRAME_RESOURCE_MGR->GetObjCBGpuAddr(mObjCBIndices[cnt]));
}

void MyFont::Render()
{
	if (!mUITexture->UIImage) {
		return;
	}

	Vec3 originPos = GetPosition();
	Vec3 fontPos = GetPosition();

	int cnt{ 0 };
	for (char ch : mText) {
		// ���������� �ʴ´ٸ� ���� ��Ʈ ����� set�� �ʿ䰡 ����.
		if (ch != ' ') {
			UpdateShaderVars(ch, cnt++);
			RESOURCE<ModelObjectMesh>("Rect")->Render();
		}

		fontPos.x += 0.07f;
		SetPosition(fontPos);
	}
	for (char ch : mScore) {
		if (ch != ' ') {
			UpdateShaderVars(ch, cnt++);
			RESOURCE<ModelObjectMesh>("Rect")->Render();
		}

		fontPos.x += 0.07f;
		SetPosition(fontPos);
	}

	SetPosition(originPos);
}
#pragma endregion





#pragma region SliderUI
SliderUI::SliderUI(const std::string& textureName, const Vec2& pos, float width, float height, rsptr<Shader> shader)
	:
	UI(textureName, pos, width, height, shader)
{
}

void SliderUI::UpdateShaderVars()
{
	mObjectCB.SliderValue = mValue;

	UI::UpdateShaderVars();
}
#pragma endregion





#pragma region Canvas
void Canvas::SetScore(int score)
{
	mFont->SetScore(std::to_string(score));
}

void Canvas::Init()
{
	mWidth = DXGIMgr::I->GetWindowWidth();
	mHeight = DXGIMgr::I->GetWindowHeight();
}

void Canvas::Update()
{
	for (auto& layer : mUIs) {
		for (auto& ui : layer) {
			ui->Update();
		}
	}
}

void Canvas::Render() const
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	for (auto& layer : mUIs) {
		for (auto& ui : layer) {
			ui->Render();
		}
	}
}

void Canvas::Clear()
{
	for (auto& layer : mUIs) {
		layer.clear();
	}
}

sptr<UI> Canvas::CreateUI(Layer layer, const std::string& texture, const Vec2& pos, float width, float height, const std::string& shader, const std::string& name)
{
	if (layer > (mkLayerCnt - 1))
		return nullptr;

	sptr<UI> ui = std::make_shared<UI>(texture, pos, width, height, RESOURCE<Shader>(shader), name);
	mUIs[layer].insert(ui);
	return ui;
}

sptr<SliderUI> Canvas::CreateSliderUI(Layer layer, const std::string& texture, const Vec2& pos, float width, float height, const std::string& shader)
{
	if (layer > (mkLayerCnt - 1))
		return nullptr;

	sptr<SliderUI> ui = std::make_shared<SliderUI>(texture, pos, width, height, RESOURCE<Shader>(shader));
	mUIs[layer].insert(ui);
	return ui;
}
void Canvas::RemoveUI(Layer layer, const std::string& name)
{
	for (auto& ui : mUIs[layer]) {
		if (ui->GetName() == name) {
			mUIs[layer].erase(ui);
			return;
		}
	}
}
void Canvas::Interact(const Vec2& mousePos)
{
	for (auto& layer : mUIs) {
		for (auto& ui : layer) {
			if (ui->CheckInteract(mousePos)) {
				ui->Interact();
			}
		}
	}
}
#pragma endregion
