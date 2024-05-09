#include "stdafx.h"
#include "Script_AbilityHolder.h"

#include "InputMgr.h"
#include "Timer.h"

#include "AbilityMgr.h"
#include "Scene.h"

void Script_AbilityHolder::Start()
{
	mAbility->SetObject(mObject);
}

void Script_AbilityHolder::Update()
{
	if (!mAbility)
		return;

	switch (mState)
	{
	case AbilityState::Ready:
		if (KEY_PRESSED(mKey)) {
			mAbility->Activate();
			mState = AbilityState::Active;
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
	}
}
