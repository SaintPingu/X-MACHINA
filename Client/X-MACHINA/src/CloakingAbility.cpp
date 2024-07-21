#include "stdafx.h"
#include "CloakingAbility.h"

#include "BattleScene.h"
#include "Object.h"
#include "Timer.h"
#include "Animator.h"

#include "Component/ParticleSystem.h"
#include "Script_Player.h"
#include "Script_AfterImageObject.h"



CloakingAbility::CloakingAbility()
	:
	Ability("Cloaking", 3.f),
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

void CloakingAbility::Start()
{
	mObject->mObjectCB.HitRimColor = Vec3{ 2.f, 1.f, 2.f };

	mAfterImage = mObject->AddComponent<Script_AfterImageObject>();
	mAfterImage->SetAfterImage(10, 1.f);
	mPrevInvokerTag = mObject->GetTag();
}

void CloakingAbility::Activate()
{
	base::Activate();
	mBuffPS = ParticleManager::I->Play("MagicCircle_Dot", mObject);

	mObject->mObjectCB.HitRimFactor = 1.f;
	mAfterImage->SetActiveUpdate(true);
	mObject->SetTag(ObjectTag::AfterSkinImage);
}

void CloakingAbility::DeActivate()
{
	base::DeActivate();
	if (mBuffPS) {
		mBuffPS->Stop();
	}

	mObject->mObjectCB.HitRimFactor = 0.f;
	mAfterImage->SetActiveUpdate(false);
	mObject->SetTag(mPrevInvokerTag);
}

bool CloakingAbility::ReducePheroAmount(bool checkOnly)
{
	sptr<Script_PheroPlayer> pheroPlayer = mObject->GetComponent<Script_PheroPlayer>();
	if (pheroPlayer) {
		return pheroPlayer->ReducePheroAmount(mPheroCost * DeltaTime());
	}

	return false;
}
