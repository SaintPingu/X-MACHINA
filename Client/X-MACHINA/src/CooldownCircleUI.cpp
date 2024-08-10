#include "stdafx.h"
#include "CooldownCircleUI.h"

#include "Component/UI.h"
#include "ResourceMgr.h"
#include "Shader.h"
#include "InputMgr.h"

CooldownCircleUI::CooldownCircleUI(const std::string& cooldownBarUI, const Vec2& pos, const Vec2& size, float maxValue)
{
	mCooldownBarUI = Canvas::I->CreateUI<SliderUI>(0, cooldownBarUI, pos, size);
	mCooldownBarUI->SetMinMaxValue(0.f, maxValue);
	mCooldownBarUI->SetShader(RESOURCE<Shader>("CooldownCircle"));
	mCooldownBarUI->SetActive(false);
}

CooldownCircleUI::~CooldownCircleUI()
{
	mCooldownBarUI->Remove();
}

void CooldownCircleUI::On()
{
	mCooldownBarUI->SetActive(true);
}

void CooldownCircleUI::Off()
{
	mCooldownBarUI->SetActive(false);
}

void CooldownCircleUI::Update(float currValue, const Vec2& pos)
{
	mCooldownBarUI->SetPosition(pos);
	mCooldownBarUI->SetValue(currValue);
}
