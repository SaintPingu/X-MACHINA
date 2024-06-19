#include "stdafx.h"
#include "IRDetectorAbility.h"

#include "Scene.h"
#include "GameFramework.h"

#include "Mesh.h"
#include "Shader.h"
#include "Object.h"
#include "Texture.h"
#include "ResourceMgr.h"

IRDetectorAbility::IRDetectorAbility()
	:
	RenderedAbility("IRDetector", 2.f, 10.f)
{
	mLayer = 0;
	mAbilityCB.Duration = 9.8f;

	mRenderedMesh = RESOURCE<ModelObjectMesh>("Rect");
	mShader = RESOURCE<Shader>("IRDetectorAbility");
	mAbilityCB.UIIndex = RESOURCE<Texture>("IRDetectorUI")->GetSrvIdx();
}

void IRDetectorAbility::Activate()
{
	base::Activate();
	Scene::I->SetFilterOptions(FilterOption::Custom);
}

void IRDetectorAbility::DeActivate()
{
	base::DeActivate();
	Scene::I->SetFilterOptions(FilterOption::Custom);
}
