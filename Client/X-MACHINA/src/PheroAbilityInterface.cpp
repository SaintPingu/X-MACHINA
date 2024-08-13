#include "stdafx.h"
#include "PheroAbilityInterface.h"

#include "Component/UI.h"
#include "ResourceMgr.h"
#include "Texture.h"
#include "CooldownCircleUI.h"
#include "Shader.h"

void IconAbilityInterface::ActiveIcon(bool active)
{
	mActiveIconUI->SetActive(active);
}

void IconAbilityInterface::SetIconUI(const std::string& background, const std::string& cooldown, const Vec2& pos, const Vec2& size, float maxValue)
{
	mAbilityIconUI = Canvas::I->CreateUI<UI>(2, cooldown, pos, size);
	mActiveIconUI = Canvas::I->CreateUI<UI>(1, "Icon_ActiveAbility", pos, size * 1.1f);
	mActiveIconUI->SetActive(false);
	mActiveIconUI->mObjectCB.UseRefract = true;
	mActiveIconUI->mObjectCB.MindRimColor = Vec3(1.f, 0.5f, 0.2f);
	mActiveIconUI->mObjectCB.LightIndex = RESOURCE<Texture>("BarDissolve")->GetSrvIdx();
	mActiveIconUI->mObjectCB.DeathElapsed = 0.3f;

	mCooldownUI = Canvas::I->CreateUI<SliderUI>(3, background, pos, size);
	mCooldownUI->SetShader(RESOURCE<Shader>("CooldownCircle"));
	mCooldownUI->SetMinMaxValue(0.f, maxValue);
}

void IconAbilityInterface::UpdateCooldownBarUI(float maxValue, float currValue)
{
	mCooldownUI->SetValue(currValue);
}
