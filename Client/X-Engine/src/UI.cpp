#include "EnginePch.h"
#include "Component/UI.h"

#include "BattleScene.h"
#include "ResourceMgr.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "FileIO.h"

#include "SoundMgr.h"
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
	mObjectCB.SliderValueX = 1.f;
	mObjectCB.SliderValueY = 1.f;

	// The object constant buffer usage flag can be set directly.
	SetUseObjCB(true);
	SetPosition(pos);

	if (scale.Length() <= FLT_EPSILON) {
		scale = Vec2(mTexture->GetWidth(), mTexture->GetHeight());
	}

	SetScale(scale);
}


bool UI::CheckHover() const
{
	if (!mIsHoverable) {
		return false;
	}

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
	if (mClickSound != "") {
		SoundMgr::I->Play("UI", mClickSound);
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
	if (!mTexture) {
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

void UI::Remove()
{
	Canvas::I->RemoveUI(this);
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

void UI::SetHover(bool val)
{
	if (mIsHover == val) {
		return;
	}

	mIsHover = val;
	if (mIsHover && mHoverSound != "") {
		SoundMgr::I->Play("UI", mHoverSound, 3.f);
	}
}

void UI::RemoveColor()
{
	mObjectCB.UseOutline = false;
}

void UI::SetColor(const Vec3& color)
{
	mObjectCB.UseOutline = true;
	mObjectCB.HitRimColor = color;
}

void UI::SetOpacity(float val)
{
	mObjectCB.AlphaIntensity = val;
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

	ProcessActiveUI([](sptr<UI> ui) {
		ui->Update();
		});
}

void Canvas::Render() const
{
	CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ProcessActiveUI([](sptr<UI> ui) {
		ui->Render();
		});
}

void Canvas::Clear()
{
	for (auto& layer : mUIs) {
		layer.clear();
	}
}

void Canvas::RemoveUI(UI* targetUI)
{
	for (auto& layer : mUIs) {
		for (auto& ui : layer) {
			if (ui.get() == targetUI) {
				layer.erase(ui);
				return;
			}
		}
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
	ProcessActiveUI([](sptr<UI> ui) {
		if (ui->IsHover()) {
			ui->OnClick();
			return;
		}
		else {
			ui->OffClick();
		}
		}, true);
}

void Canvas::CheckHover() const
{
	bool hovered = false;
	ProcessActiveUI([&hovered](sptr<UI> ui) {
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
		}, true);
}

void Canvas::ProcessActiveUI(std::function<void(sptr<UI>)> processFunc, bool isReverse) const
{
	if (isReverse) {
		for (auto& layer : mUIs | std::views::reverse) {
			for (auto& ui : layer) {
				if (ui->IsActive()) {
					processFunc(ui);
				}
			}
		}
	}
	else {
		for (auto& layer : mUIs) {
			for (auto& ui : layer) {
				if (ui->IsActive()) {
					processFunc(ui);
				}
			}
		}
	}
}

void Canvas::ProcessAllUI(std::function<void(sptr<UI>)> processFunc, bool isReverse) const
{
	if (isReverse) {
		for (auto& layer : mUIs | std::views::reverse) {
			for (auto& ui : layer) {
				processFunc(ui);
			}
		}
	}
	else {
		for (auto& layer : mUIs) {
			for (auto& ui : layer) {
				processFunc(ui);
			}
		}
	}
}
void Canvas::ProcessKeyboardMsg(UINT message, WPARAM wParam, LPARAM lParam)
{
	for (auto& layer : mUIs) {
		for (auto& ui : layer) {
			ui->ProcessKeyboardMsg(message, wParam, lParam);
		}
	}
}
#pragma endregion