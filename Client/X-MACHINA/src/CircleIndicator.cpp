#include "stdafx.h"
#include "CircleIndicator.h"

#include "BattleScene.h"
#include "GameFramework.h"

#include "Mesh.h"
#include "Shader.h"
#include "Object.h"
#include "Texture.h"
#include "ResourceMgr.h"

AttackIndicator::AttackIndicator(float activeTime, std::string indicatorShape)
	:
	RenderedAbility("IRDetector", 0.f, activeTime)
{
	mLayer = 0;

	mRenderedObject = std::make_shared<GameObject>();
	mRenderedObject->SetModel(indicatorShape);

	mShader = RESOURCE<Shader>("CircleIndicator");
}

void AttackIndicator::Update(float activeTime)
{
	base::Update(activeTime);
}

void AttackIndicator::Activate()
{
	base::Activate();

	const Vec3 playerPos = mObject->GetPosition() + Vec3{ 0.f, 0.5f, 0.f };
	const Quat playerRot = mObject->GetRotation();
	mRenderedObject->SetPosition(playerPos);
	mRenderedObject->SetLocalRotation(playerRot);
}

void AttackIndicator::DeActivate()
{
	base::DeActivate();
}
