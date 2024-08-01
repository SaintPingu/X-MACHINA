#include "stdafx.h"
#include "Script_AbilityHolder.h"

#include "InputMgr.h"
#include "Timer.h"

#include "AbilityMgr.h"
#include "BattleScene.h"
#include "Script_Player.h"

#pragma region Default
void Script_AbilityHolder::SetAbility(sptr<Ability> ability)
{
	mAbility = ability;
}

const std::string& Script_AbilityHolder::GetAbilityName() const
{
	return mAbility->GetName();
}

void Script_AbilityHolder::Start()
{
	base::Start();

	mPlayer = mObject->GetComponent<Script_Player>();
	mAbility->SetObject(mObject);
	mAbility->Start();
	mAbility->SetTerminateCallback(std::bind_back(&Script_AbilityHolder::Terminate, this));
}

void Script_AbilityHolder::Update()
{
	if (!mAbility)
		return;

	switch (mState)
	{
	case AbilityState::Ready:
		break;
	case AbilityState::Active:
		mActiveTime -= DeltaTime();
		if (mActiveTime <= 0) {
			mActiveTime = 0;
			Disable();
		}
		mAbility->Update(mActiveTime);
		break;
	case AbilityState::Cooldown:
		if (mCooldownTime > 0.f) {
			mCooldownTime -= DeltaTime();
		}
		else {
			mState = AbilityState::Ready;
		}
		break;
	default:
		break;
	}
}

void Script_AbilityHolder::Toggle()
{
	if (mState == AbilityState::Ready) {
		Enable();
	}
	else if (mState == AbilityState::Active) {
		Disable();
	}
}

void Script_AbilityHolder::Terminate()
{
	mState = AbilityState::Ready;
}

void Script_AbilityHolder::Enable()
{
	if (mState != AbilityState::Ready) {
		return;
	}

	mState = AbilityState::Active;
	mAbility->Activate();
	mActiveTime = mAbility->GetActiveTime();
}

void Script_AbilityHolder::Disable()
{
	if (mState != AbilityState::Active) {
		return;
	}

	mState = AbilityState::Cooldown;
	mAbility->DeActivate();
	mCooldownTime = mAbility->GetCooldownTime();
}
#pragma endregion





#pragma region Toggle
void Script_ToggleAbilityHolder::Update()
{
	if (!mAbility)
		return;

	switch (mState)
	{
	case AbilityState::Ready:
		break;
	case AbilityState::Active:
		mAbility->Update(0.f);
		break;
	case AbilityState::Cooldown:
		if (mCooldownTime > 0.f) {
			mCooldownTime -= DeltaTime();
		}
		else {
			mState = AbilityState::Ready;
		}
		break;
	default:
		break;
	}
}
#pragma endregion


void Script_CooldownAbilityHolder::Toggle()
{
	if (mState == AbilityState::Ready) {
		Enable();
	}
}
