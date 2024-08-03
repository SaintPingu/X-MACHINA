#include "stdafx.h"
#include "Script_Ability_AerialController.h"

#include "BattleScene.h"
#include "ResourceMgr.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

void Script_Ability_AerialController::Awake()
{
	base::Awake();

	Init("AerialController", 10.f, 30.f);

	mLayer = 0;
	mAbilityCB.Duration = 29.8f;

	mRenderedMesh = RESOURCE<ModelObjectMesh>("Rect");
	mShader = RESOURCE<Shader>("AerialController");
	mAbilityCB.UIIndex = RESOURCE<Texture>("AerialControllerUI")->GetSrvIdx();
	mAbilityCB.NoiseIndex = RESOURCE<Texture>("Noise")->GetSrvIdx();
	mAbilityCB.UI2Index = RESOURCE<Texture>("FullNoise")->GetSrvIdx();
}

void Script_Ability_AerialController::On()
{
	base::On();
	BattleScene::I->SetFilterOptions(FilterOption::Custom);
}

void Script_Ability_AerialController::Off()
{
	base::Off();
	BattleScene::I->SetFilterOptions(FilterOption::Custom);
}
