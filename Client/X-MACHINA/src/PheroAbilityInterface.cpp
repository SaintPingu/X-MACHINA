#include "stdafx.h"
#include "PheroAbilityInterface.h"

#include "Component/UI.h"
#include "ResourceMgr.h"
#include "CooldownCircleUI.h"
#include "Shader.h"

void IconAbilityInterface::SetIconUI(const std::string& background, const std::string& cooldown, const Vec2& pos, const Vec2& size, float maxValue)
{
	mAbilityIconUI = Canvas::I->CreateUI<UI>(2, cooldown, pos, size);

	mCooldownUI = Canvas::I->CreateUI<SliderUI>(3, background, pos, size);
	mCooldownUI->SetShader(RESOURCE<Shader>("CooldownCircle"));
	mCooldownUI->SetMinMaxValue(0.f, maxValue);
}

void IconAbilityInterface::UpdateCooldownBarUI(float maxValue, float currValue)
{
	mCooldownUI->SetValue(currValue);
}
