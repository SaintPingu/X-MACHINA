#include "stdafx.h"
#include "ShieldAbility.h"

#include "Scene.h"
#include "GameFramework.h"

#include "Mesh.h"
#include "Shader.h"
#include "Object.h"
#include "ResourceMgr.h"

ShieldAbility::ShieldAbility(float cooldownTime, float activeTime)
	:
	RenderedAbility(cooldownTime, activeTime)
{
	mRenderedObject = std::make_shared<GameObject>();
	mRenderedObject->SetModel("Shield");
	mShader = RESOURCE<Shader>("ShieldAbility");
}

void ShieldAbility::Activate()
{
	base::Activate();

	// 어빌리티 매니저에 삽입되자마자 렌더링되기 때문에 호출해줘야 한다.
	FollowObject();
}


void ShieldAbility::Update()
{
	FollowObject();
}

void ShieldAbility::FollowObject()
{
	const Vec3 playerPos = mObject->GetPosition() + Vec3{ 0.f, 0.85f, 0.f };
	mRenderedObject->SetPosition(playerPos);
}
