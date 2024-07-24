#include "stdafx.h"
#include "CircleIndicator.h"

#include "BattleScene.h"
#include "GameFramework.h"

#include "Mesh.h"
#include "Shader.h"
#include "Object.h"
#include "Texture.h"
#include "ResourceMgr.h"

CircleIndicator::CircleIndicator(float activeTime)
	:
	RenderedAbility("IRDetector", 0.f, activeTime)
{
	mLayer = 0;

	mRenderedObject = std::make_shared<GameObject>();
	mRenderedObject->SetModel("CircleIndicator");

	mShader = RESOURCE<Shader>("CircleIndicator");
}

void CircleIndicator::Update(float activeTime)
{
	base::Update(activeTime);
}

void CircleIndicator::Activate()
{
	base::Activate();

	const Vec3 playerPos = mObject->GetPosition() + Vec3{ 0.f, 0.5f, 0.f };
	mRenderedObject->SetPosition(playerPos);
}

void CircleIndicator::DeActivate()
{
	base::DeActivate();
}
