#include "stdafx.h"
#include "IRDetectorAbility.h"

#include "Scene.h"
#include "GameFramework.h"

#include "Mesh.h"
#include "Shader.h"
#include "Object.h"
#include "ResourceMgr.h"

IRDetectorAbility::IRDetectorAbility(float cooldownTime, float activeTime)
	:
	RenderedAbility(cooldownTime, activeTime)
{
	mLayer = 0;
	mRenderedMesh = RESOURCE<ModelObjectMesh>("Rect");
	mShader = RESOURCE<Shader>("IRDetectorAbility");
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
