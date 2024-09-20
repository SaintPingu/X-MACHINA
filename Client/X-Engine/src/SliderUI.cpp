#include "EnginePch.h"
#include "Component/UI.h"

#include "ResourceMgr.h"
#include "Texture.h"

SliderUI::SliderUI(const std::string& textureName, const Vec2& pos, Vec2 scale)
	:
	UI(textureName, pos, scale)
{
}

void SliderUI::SetDissolve(const Vec3& color)
{
	mObjectCB.LightIndex = RESOURCE<Texture>("BarDissolve")->GetSrvIdx();
	mObjectCB.UseRefract = true;
	mObjectCB.MindRimColor = color;
	mObjectCB.DeathElapsed = 0.03f;
}

void SliderUI::UpdateShaderVars(rsptr<Texture> texture)
{
	mObjectCB.SliderValueX = mValue;

	UI::UpdateShaderVars(mTexture);
}