#include "EnginePch.h"
#include "Component/UI.h"

#include "ResourceMgr.h"
#include "Texture.h"
#include "SoundMgr.h"
#include "InputMgr.h"

Button::Button(const std::string& textureName, const Vec2& pos, Vec2 scale)
	:
	UI(textureName, pos, scale)
{
	mCrntTexture = mTexture;
	SetHoverable(true);
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