#include "stdafx.h"
#include "CloakingAbility.h"

#include "Scene.h"
#include "Object.h"
#include "Timer.h"

#include "Script_Player.h"


CloakingAbility::CloakingAbility()
	:
	Ability("Cloaking"),
	PheroAbilityInterface(30.f)
{
}

void CloakingAbility::Update(float activeTime)
{
	if (!ReducePheroAmount()) {
		mTerminateCallback();
		DeActivate();
		return;
	}
}

void CloakingAbility::Activate()
{
	base::Activate();

	DynamicEnvironmentMappingManager::I->AddObject(mObject);
}

void CloakingAbility::DeActivate()
{
	base::DeActivate();

	DynamicEnvironmentMappingManager::I->RemoveObject(mObject);
}

bool CloakingAbility::ReducePheroAmount()
{
	sptr<Script_PheroPlayer> pheroPlayer = mObject->GetComponent<Script_PheroPlayer>();
	if (pheroPlayer) {
		return pheroPlayer->ReducePheroAmount(mPheroCost * DeltaTime());
	}

	return false;
}
