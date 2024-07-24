#include "EnginePch.h"
#include "Component/UI.h"

#include "BattleScene.h"
#include "ResourceMgr.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "FileIO.h"

#include "DXGIMgr.h"
#include "InputMgr.h"
#include "FrameResource.h"
#include "MultipleRenderTarget.h"


#pragma region UITexture
UITexture::UITexture(const std::string& imageName, float width, float height)
{
	Image = RESOURCE<Texture>(imageName);
	Width = width / Canvas::I->GetWidth();
	Height = height / Canvas::I->GetHeight();
}
#pragma endregion


#pragma region UI
UI::UI(const std::string& textureName, Vec2 pos, float width, float height)
	:
	Object()
{
	mTexture = std::make_shared<UITexture>(textureName, width, height);
	mObjectCB.SliderValue = 1.f;

	// ������Ʈ ��� ���� ��� �÷��״� ���� ������ �� �ִ�.
	SetUseObjCB(true);
	SetPosition(pos);
}

void UI::ChangeUITexture(rsptr<UITexture> newUITexture)
{
	mTexture = newUITexture;
}
bool UI::CheckClick(const Vec2& mousePos)
{
	Vec2 pos = GetPosition();
	float width = mTexture->Width;
	float height = mTexture->Height;

	float left = pos.x - width;
	float right = pos.x + width;
	float up = pos.y + height;
	float down = pos.y - height;

	if (mousePos.x < left) {
		return false;
	}
	if (mousePos.x > right) {
		return false;
	}
	if (mousePos.y < down) {
		return false;
	}
	if (mousePos.y > up) {
		return false;
	}

	return true;
}

void UI::OnClick()
{
	base::OnClick();

	if (mClickCallback) {
		mClickCallback();
	}
}


void UI::UpdateShaderVars()
{
	mObjectCB.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mTexture->Width, mTexture->Height, 1.f), _MATRIX(GetWorldTransform())));
	mObjectCB.MatIndex = mTexture->Image->GetSrvIdx();

	FRAME_RESOURCE_MGR->CopyData(mObjCBIndices.front(), mObjectCB);
	DXGIMgr::I->SetGraphicsRootConstantBufferView(RootParam::Object, FRAME_RESOURCE_MGR->GetObjCBGpuAddr(mObjCBIndices.front()));
}

void UI::Render()
{
	if (!mIsActive) {
		return;
	}

	if (!mTexture->Image) {
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

void UI::SetColor(const Vec3& color)
{
	mObjectCB.UseOutline = true;
	mObjectCB.HitRimColor = color;
}

#pragma endregion






#pragma region SliderUI
SliderUI::SliderUI(const std::string& textureName, const Vec2& pos, float width, float height)
	:
	UI(textureName, pos, width, height)
{
}

void SliderUI::UpdateShaderVars()
{
	mObjectCB.SliderValue = mValue;

	UI::UpdateShaderVars();
}
#pragma endregion





#pragma region Canvas
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

void Canvas::RemoveUI(Layer layer, UI* targetUI)
{
	for (auto& ui : mUIs[layer]) {
		if (ui.get() == targetUI) {
			mUIs[layer].erase(ui);
			return;
		}
	}
}

void Canvas::CheckClick(const Vec2& mousePos)
{
	for (auto& layer : mUIs) {
		for (auto& ui : layer) {
			if (ui->CheckClick(mousePos)) {
				ui->OnClick();
				return;
			}
		}
	}
}
#pragma endregion

Button::Button(const std::string& textureName, Vec2 pos, float width, float height)
	:
	UI(textureName, pos, width, height)
{

}