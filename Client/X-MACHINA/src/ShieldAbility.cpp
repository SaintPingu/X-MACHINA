#include "stdafx.h"
#include "ShieldAbility.h"

#include "Mesh.h"
#include "Shader.h"
#include "ResourceMgr.h"
#include "Object.h"
#include "Scene.h"
#include "GameFramework.h"

ShieldAbility::ShieldAbility(float cooldownTime, float activeTime)
	:
	RenderedAbility(cooldownTime, activeTime)
{
	mRenderedObject = std::make_shared<GameObject>();
	mRenderedObject->SetModel("Sphere");
	mShader = RESOURCE<Shader>("Transparent");
}

void ShieldAbility::Update()
{
	mRenderedObject->SetPosition(mObject->GetPosition());
}
