#include "stdafx.h"
#include "Script_Ability_IRDetector.h"

#include "BattleScene.h"
#include "ResourceMgr.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"


void Script_Ability_IRDetector::Awake()
{
	base::Awake();

	base::Init("IRDetetor", 2.f, 10.f);

	mLayer = 0;
	mAbilityCB.Duration = 9.8f;

	mRenderedMesh = RESOURCE<ModelObjectMesh>("Rect");
	mShader = RESOURCE<Shader>("IRDetectorAbility");
	mAbilityCB.UIIndex = RESOURCE<Texture>("IRDetectorUI")->GetSrvIdx();
	mAbilityCB.NoiseIndex = RESOURCE<Texture>("Noise")->GetSrvIdx();
}

void Script_Ability_IRDetector::On()
{
	base::On();
	BattleScene::I->SetFilterOptions(FilterOption::Custom);
}

void Script_Ability_IRDetector::Off()
{
	base::Off();
	BattleScene::I->SetFilterOptions(FilterOption::Custom);
}
