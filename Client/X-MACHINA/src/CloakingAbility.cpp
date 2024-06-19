#include "stdafx.h"
#include "CloakingAbility.h"

#include "Scene.h"
#include "Object.h"
#include "Timer.h"

#include "Script_Player.h"
#include "Component/ParticleSystem.h"


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
	mBuffSparkPS = ParticleManager::I->Play("MagicCircle_Sparks", mObject);
	mBuffDotPS = ParticleManager::I->Play("MagicCircle_Dot", mObject);

	DynamicEnvironmentMappingManager::I->AddObject(mObject);
}

void CloakingAbility::DeActivate()
{
	base::DeActivate();
	mBuffSparkPS->Stop();
	mBuffDotPS->Stop();

	DynamicEnvironmentMappingManager::I->RemoveObject(mObject);
}

bool CloakingAbility::ReducePheroAmount(bool checkOnly)
{
	sptr<Script_PheroPlayer> pheroPlayer = mObject->GetComponent<Script_PheroPlayer>();
	if (pheroPlayer) {
		return pheroPlayer->ReducePheroAmount(mPheroCost * DeltaTime());
	}

	return false;
}
