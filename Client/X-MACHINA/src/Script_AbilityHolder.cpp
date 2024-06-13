#include "stdafx.h"
#include "Script_AbilityHolder.h"

#include "InputMgr.h"
#include "Timer.h"

#include "AbilityMgr.h"
#include "Scene.h"


#pragma region Default
void Script_AbilityHolder::Start()
{
	mAbility->SetObject(mObject);
	mAbility->SetTerminateCallback(std::bind_back(&Script_AbilityHolder::Terminate, this));
}

void Script_AbilityHolder::Update()
{
	if (!mAbility)
		return;

	switch (mState)
	{
	case AbilityState::Ready:
		if (KEY_TAP(mKey)) {
			mState = AbilityState::Active;
			mAbility->Activate();
			mActiveTime = mAbility->GetActiveTime();
		}
		break;
	case AbilityState::Active:
		if (mActiveTime > 0.f) {
			mActiveTime -= DeltaTime();
			mAbility->Update(mActiveTime);
		}
		else {
			mAbility->DeActivate();
			mState = AbilityState::Cooldown;
			mCooldownTime = mAbility->GetCooldownTime();
		}
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

void Script_AbilityHolder::Terminate()
{
	mState = AbilityState::Ready;
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
		if (KEY_TAP(mKey)) {
			mState = AbilityState::Active;
			mAbility->Activate();
		}
		break;
	case AbilityState::Active:
		if (mAbility->IsToggleAbility() && KEY_TAP(mKey)) {
			mState = AbilityState::Ready;
			mAbility->DeActivate();
		}
		else{
			mAbility->Update(0.f);
		}
		break;
	default:
		break;
	}
}
#pragma endregion
