#include "stdafx.h"
#include "Script_Ability_NightVision.h"

#include "BattleScene.h"
#include "ResourceMgr.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"


void Script_Ability_NightVision::Awake()
{
	base::Awake();

	Init("NightVision", 2.f, 60.f);

	mLayer = 0;
	mAbilityCB.Duration = 59.8f;

	mRenderedMesh = RESOURCE<ModelObjectMesh>("Rect");
	mShader = RESOURCE<Shader>("NightVisionAbility");
	mAbilityCB.UIIndex = RESOURCE<Texture>("NightVisionUI")->GetSrvIdx();
	mAbilityCB.NoiseIndex = RESOURCE<Texture>("Noise")->GetSrvIdx();
}

void Script_Ability_NightVision::On()
{
	base::On();
	BattleScene::I->SetFilterOptions(FilterOption::Custom);
}

void Script_Ability_NightVision::Off()
{
	base::Off();
	BattleScene::I->SetFilterOptions(FilterOption::Custom);
}
