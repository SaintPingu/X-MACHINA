#include "stdafx.h"
#include "Script_AbilityHolder.h"

#include "InputMgr.h"
#include "Timer.h"

#include "AbilityMgr.h"
#include "BattleScene.h"
#include "Script_Player.h"

#pragma region Default
void Script_AbilityHolder::SetAbility(int key, sptr<Ability> ability)
{
	mKey = key;
	mAbility = ability;
	mAbility->SetHolderKey(key);
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

	if (mPlayer->IsActiveChatBox()) {
		return;
	}

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

	if (mPlayer->IsActiveChatBox()) {
		return;
	}

	switch (mState)
	{
	case AbilityState::Ready:
		if (KEY_TAP(mKey)) {
			mState = AbilityState::Active;
			mAbility->Activate();
		}
		break;
	case AbilityState::Active:
		if (KEY_TAP(mKey)) {
			mState = AbilityState::Cooldown;
			mAbility->DeActivate();
			mCooldownTime = mAbility->GetCooldownTime();
		}
		else{
			mAbility->Update(0.f);
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
#pragma endregion

void Script_TriggerAbilityHolder::Update()
{
	if (!mAbility)
		return;

	switch (mState)
	{
	case AbilityState::Ready:
		if (mIsOnTrigger) {
			mIsOnTrigger = false;
			mState = AbilityState::Active;
			mAbility->Activate();
			mActiveTime = mAbility->GetActiveTime();
		}
		break;
	case AbilityState::Active:
		if (mIsOnTrigger) {
			mIsOnTrigger = false;
			mState = AbilityState::Cooldown;
			mAbility->DeActivate();
			mCooldownTime = mAbility->GetCooldownTime();
		}
		else {
			mActiveTime -= DeltaTime();
			mAbility->Update(mActiveTime);
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
