#include "stdafx.h"
#include "CloakingAbility.h"

#include "Scene.h"
#include "Object.h"
#include "Timer.h"

#include "Script_Player.h"
#include "Component/ParticleSystem.h"

#include "Animator.h"


CloakingAbility::CloakingAbility()
	:
	Ability("Cloaking"),
	PheroAbilityInterface(30.f)
{
	mAfterImageObject = std::dynamic_pointer_cast<GameObject>(Scene::I->Instantiate("EliteTrooper", ObjectTag::Unspecified));
	mAfterImageObject->SetUseShadow(false);
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

	mAfterImageObject->SetPosition(mObject->GetPosition() + Vec3{ 0.f, 0.f, 0.f });
	mAfterImageObject->SetLocalRotation(mObject->GetLocalRotation());
	mAfterImageObject->GetAnimator()->CloneBoneFrames(mObject);
}

void CloakingAbility::DeActivate()
{
	base::DeActivate();
	mBuffSparkPS->Stop();
	mBuffDotPS->Stop();
}

bool CloakingAbility::ReducePheroAmount(bool checkOnly)
{
	sptr<Script_PheroPlayer> pheroPlayer = mObject->GetComponent<Script_PheroPlayer>();
	if (pheroPlayer) {
		return pheroPlayer->ReducePheroAmount(mPheroCost * DeltaTime());
	}

	return false;
}
