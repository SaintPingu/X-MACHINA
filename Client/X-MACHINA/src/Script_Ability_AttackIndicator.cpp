#include "stdafx.h"
#include "Script_Ability_AttackIndicator.h"

#include "BattleScene.h"
#include "GameFramework.h"

#include "Mesh.h"
#include "Shader.h"
#include "Object.h"
#include "Texture.h"
#include "ResourceMgr.h"


void Script_Ability_AttackIndicator::Init(float activeTime, const std::string& indicatorShape)
{
	base::Init("AttackIndicator", 0.f, activeTime);

	mLayer = 0;
	mRenderedObject = std::make_shared<GameObject>();
	mRenderedObject->SetModel(indicatorShape);

	mShader = RESOURCE<Shader>("CircleIndicator");
	SetType(Type::Toggle);
}


void Script_Ability_AttackIndicator::On()
{
	base::On();

	const Vec3 playerPos = mObject->GetPosition() + Vec3{ 0.f, 0.5f, 0.f };
	const Quat playerRot = mObject->GetRotation();
	if (mRenderedObject) {
		mRenderedObject->SetPosition(playerPos);
		mRenderedObject->SetLocalRotation(playerRot);
	}
}
