#include "stdafx.h"
#include "Script_AimController.h"

#include "Component/UI.h"

#include "TextMgr.h"
#include "InputMgr.h"
#include "GameFramework.h"

void Script_AimController::Awake()
{
	base::Awake();

	{
		TextOption textOption;
		textOption.FontSize = 10.f;
		textOption.FontColor = TextFontColor::Type::Red;
		textOption.FontWeight = TextFontWeight::THIN;
		textOption.VAlignment = TextParagraphAlign::Near;
		textOption.HAlignment = TextAlignType::Trailing;

		//mPosText = TextMgr::I->CreateText("(1920, 1080)", Vec2(-10, -10), textOption);
	}
}

void Script_AimController::Update()
{
	base::Update();

	if (mUI) {
		mUI->SetPosition(InputMgr::I->GetMousePos());
	}

	UpdatePosText();
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
	//if (!mPosText) {
	//	return;
	//}

	//Vec2 mousePos = InputMgr::I->GetMousePos();
	//std::string x = std::to_string(static_cast<int>(mousePos.x));
	//std::string y = std::to_string(static_cast<int>(mousePos.y));

	//std::string pX = std::to_string(static_cast<int>(mObject->GetPosition().x));
	//std::string pZ = std::to_string(static_cast<int>(mObject->GetPosition().z));


	//mPosText->SetText("(Pos : " + pX  +  ", " + pZ + ") (Dir : " + x + ", " + y + ")");
}
