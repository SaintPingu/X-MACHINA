#include "stdafx.h"
#include "Script_AimController.h"

#include "Component/UI.h"

#include "TextMgr.h"
#include "InputMgr.h"
#include "GameFramework.h"

void Script_AimController::Awake()
{
	base::Awake();

	RESOLUTION resolution = GameFramework::I->GetWindowResolution();
	mMaxPos.x = resolution.Width - 10.f;
	mMaxPos.y = resolution.Height - 30.f;

	{
		TextOption textOption;
		textOption.FontSize = 10.f;
		textOption.FontColor = TextFontColor::Type::Red;
		textOption.FontWeight = TextFontWeight::THIN;
		textOption.VAlignment = TextParagraphAlign::Near;
		textOption.HAlignment = TextAlignType::Trailing;

		mPosText = TextMgr::I->CreateText("(1920, 1080)", Vec2(-10, -10), textOption);
	}
}

void Script_AimController::Update()
{
	base::Update();

	const Vec2 mouseDelta = InputMgr::I->GetMouseDelta() * mouseSensitivity;

	mMousePos += mouseDelta;
	mMousePos.x = std::clamp(mMousePos.x, -mMaxPos.x, mMaxPos.x);
	mMousePos.y = std::clamp(mMousePos.y, -mMaxPos.y, mMaxPos.y);

	if (mUI) {
		mUI->SetPosition(mMousePos);
	}

	UpdatePosText();
}

Vec2 Script_AimController::GetAimNDCPos() const
{
	return Vec2(mMousePos.x / Canvas::I->GetWidth(), mMousePos.y / Canvas::I->GetHeight());
}

Vec2 Script_AimController::GetScreenAimPos() const
{
	float windowWidth = static_cast<float>(GameFramework::I->GetWindowResolution().Width);
	float windowHeight = static_cast<float>(GameFramework::I->GetWindowResolution().Height);

	Vec2 screenPos = mMousePos / 2.f;
	screenPos.y = 1.f - screenPos.y;
	screenPos += Vec2{ windowWidth, windowHeight } / 2.f;

	return screenPos;
}

sptr<Texture> Script_AimController::GetTexture() const
{
	assert(mUI);
	return mUI->GetTexture();
}

Vec2 Script_AimController::GetTextureScale() const
{
	return mUI->GetScale();
}


void Script_AimController::ChangeAimTexture(rsptr<Texture> newTexture, const Vec2& scale)
{
	if (mUI) {
		mUI->ChangeTexture(newTexture);
		mUI->SetScale(scale);
	}
}

void Script_AimController::UpdatePosText()
{
	if (!mPosText) {
		return;
	}

	std::string x = std::to_string(static_cast<int>(mMousePos.x));
	std::string y = std::to_string(static_cast<int>(mMousePos.y));

	mPosText->SetText("(" + x + ", " + y + ")");
}
