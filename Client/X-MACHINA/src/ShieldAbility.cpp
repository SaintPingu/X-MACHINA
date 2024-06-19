#include "stdafx.h"
#include "ShieldAbility.h"

#include "Scene.h"
#include "GameFramework.h"

#include "Mesh.h"
#include "Shader.h"
#include "Object.h"
#include "ResourceMgr.h"

#include "Script_LiveObject.h"
#include "Script_Player.h"

ShieldAbility::ShieldAbility()
	:
	RenderedAbility("Shield", 2.f, 4.5f),
	PheroAbilityInterface(100.f)
{
	mLayer = 1;
	mAbilityCB.Duration = 4.f;
	mShieldAmount = 30.f;
	mRenderedObject = std::make_shared<GameObject>();
	mRenderedObject->SetModel("Shield");
	
	mShader = RESOURCE<Shader>("ShieldAbility");
}

void ShieldAbility::Update(float activeTime)
{
	base::Update(activeTime);

	const Vec3 playerPos = mObject->GetPosition() + Vec3{ 0.f, 0.85f, 0.f };
	mRenderedObject->SetPosition(playerPos);
}

void ShieldAbility::Activate()
{
	if (!ReducePheroAmount()) {
		mTerminateCallback();
		return;
	}

	base::Activate();

	mObject->GetComponent<Script_LiveObject>()->SetShield(mShieldAmount);
}

void ShieldAbility::DeActivate()
{
	base::DeActivate();

	mObject->GetComponent<Script_LiveObject>()->SetShield(0.f);
}

bool ShieldAbility::ReducePheroAmount(bool checkOnly)
{
	sptr<Script_PheroPlayer> pheroPlayer = mObject->GetComponent<Script_PheroPlayer>();
	if (pheroPlayer) {
		return pheroPlayer->ReducePheroAmount(mPheroCost);
	}

	return false;
}
