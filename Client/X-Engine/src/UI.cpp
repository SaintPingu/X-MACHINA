#include "UI.h"
#include "UI.h"
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



#pragma region UI
UI::UI(const std::string& textureName, const Vec2& pos, const Vec2& scale)
	:
	Object()
{
	mTexture = RESOURCE<Texture>(textureName);
	mObjectCB.SliderValue = 1.f;

	// ������Ʈ ��� ���� ��� �÷��״� ���� ������ �� �ִ�.
	SetUseObjCB(true);
	SetPosition(pos);
	SetScale(scale);
}


bool UI::CheckClick(const Vec2& mousePos)
{
	Vec2 pos = GetPosition();
	float width = mScale.x;
	float height = mScale.y;

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


void UI::UpdateShaderVars(rsptr<Texture> texture)
{
	mObjectCB.MtxWorld = XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(mScale.x, mScale.y, 1.f), _MATRIX(GetWorldTransform())));
	mObjectCB.MatIndex = texture->GetSrvIdx();

	FRAME_RESOURCE_MGR->CopyData(mObjCBIndices.front(), mObjectCB);
	DXGIMgr::I->SetGraphicsRootConstantBufferView(RootParam::Object, FRAME_RESOURCE_MGR->GetObjCBGpuAddr(mObjCBIndices.front()));
}

void UI::Render()
{
	if (!mIsActive || !mTexture) {
		return;
	}

	if (mShader) {
		mShader->Set();
	}
	else {
		RESOURCE<Shader>("Canvas")->Set();
	}

	UpdateShaderVars(mTexture);

	RESOURCE<ModelObjectMesh>("Rect")->Render();
}

Vec2 UI::GetScale() const
{
	return Vec2(mScale.x * Canvas::I->GetWidth(), mScale.y * Canvas::I->GetHeight());
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

void UI::SetScale(const Vec2& scale)
{
	mScale.x = scale.x / Canvas::I->GetWidth();
	mScale.y = scale.y / Canvas::I->GetHeight();
}

void UI::SetColor(const Vec3& color)
{
	mObjectCB.UseOutline = true;
	mObjectCB.HitRimColor = color;
}

#pragma endregion






#pragma region SliderUI
SliderUI::SliderUI(const std::string& textureName, const Vec2& pos, const Vec2& scale)
	:
	UI(textureName, pos, scale)
{
}

void SliderUI::UpdateShaderVars(rsptr<Texture> texture)
{
	mObjectCB.SliderValue = mValue;

	UI::UpdateShaderVars(mTexture);
}
#pragma endregion




#pragma region Button
Button::Button(const std::string& textureName, Vec2 pos, const Vec2& scale)
	:
	UI(textureName, pos, scale)
{
	mCrntTexture = mTexture;
}

void Button::Render()
{
	base::Render();

	if (mClicked) {
		if (!KEY_PRESSED(VK_LBUTTON)) {
			mClicked = false;
			if (mPressedTexture) {
				mCrntTexture = mTexture;
			}
		}
	}
}

void Button::SetHighlightTexture(const std::string& textureName)
{
	mHighlightTexture = RESOURCE<Texture>(textureName);
}

void Button::SetPressedTexture(const std::string& textureName)
{
	mPressedTexture = RESOURCE<Texture>(textureName);
}

void Button::SetDisabledTexture(const std::string& textureName)
{
	mDisabledTexture = RESOURCE<Texture>(textureName);
}

void Button::UpdateShaderVars(rsptr<Texture> texture)
{
	UI::UpdateShaderVars(mCrntTexture);
}

void Button::OnClick()
{
	base::OnClick();

	mClicked = true;
	if (mPressedTexture) {
		mCrntTexture = mPressedTexture;
	}
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
