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
UI::UI(const std::string& textureName, const Vec2& pos, Vec2 scale)
	:
	Object()
{
	mTexture = RESOURCE<Texture>(textureName);
	mObjectCB.SliderValue = 1.f;

	// ������Ʈ ��� ���� ��� �÷��״� ���� ������ �� �ִ�.
	SetUseObjCB(true);
	SetPosition(pos);

	if (scale.Length() <= FLT_EPSILON) {
		scale = Vec2(mTexture->GetWidth(), mTexture->GetHeight());
	}

	SetScale(scale);
}


bool UI::CheckHover() const
{
	const Vec2 mousePos = InputMgr::I->GetMouseNDCPos();
	const Vec2 pos = GetPosition();
	const float width = mScale.x;
	const float height = mScale.y;

	const float left = pos.x - width;
	const float right = pos.x + width;
	const float up = pos.y + height;
	const float down = pos.y - height;

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

void UI::SetPosition(float x, float y)
{
	x /= (Canvas::I->GetWidth() * 0.5f);
	y /= (Canvas::I->GetHeight() * 0.5f);
	Transform::SetPosition(x, y, 0.f);
}

void UI::SetPosition(const Vec2& pos)
{
	SetPosition(pos.x, pos.y);
}

void UI::SetPosition(const Vec3& pos)
{
	SetPosition(pos.x, pos.y);
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
SliderUI::SliderUI(const std::string& textureName, const Vec2& pos, Vec2 scale)
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
Button::Button(const std::string& textureName, const Vec2& pos, Vec2 scale)
	:
	UI(textureName, pos, scale)
{
	mCrntTexture = mTexture;
}

void Button::Update()
{
	base::Update();

	if (mHighlightTexture) {
		if (mIsHover) {
			mCrntTexture = mHighlightTexture;
		}
		else {
			mCrntTexture = mTexture;
		}
	}
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
	CheckHover();

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

void Canvas::CheckClick() const
{
	for (auto& layer : mUIs) {
		for (auto& ui : layer) {
			if (ui->IsHover()) {
				ui->OnClick();
				return;
			}
		}
	}
}

void Canvas::CheckHover() const
{
	bool hovered = false;
	for (auto& layer : mUIs) {
		for (auto& ui : layer) {
			if (ui->CheckHover()) {
				if (!hovered) {
					hovered = true;
					ui->SetHover(true);
				}
				else {
					ui->SetHover(false);
				}
			}
			else {
				ui->SetHover(false);
			}
		}
	}
}
#pragma endregion
