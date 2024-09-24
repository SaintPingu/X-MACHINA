#include "stdafx.h"
#include "Script_Player.h"

#include "SliderBarUI.h"
#include "Timer.h"





#pragma region Player
void Script_PheroPlayer::Awake()
{
	base::Awake();

	mStartPheroAmount = 200.f;
	mCurPheroAmount = mStartPheroAmount;
	mMaxPheroAmount = 1000.f;
	mPheroRegenRate = 10.f; // 초당 페로 회복량 (초당 3회복)
}

void Script_PheroPlayer::Update()
{
	base::Update();

	AddPheroAmount(DeltaTime() * mPheroRegenRate);
}

void Script_PheroPlayer::Respawn()
{
	Script_Player::Respawn();

	mCurPheroAmount = mStartPheroAmount;
}

void Script_PheroPlayer::AddPheroAmount(float pheroAmount)
{
	mCurPheroAmount = min(mCurPheroAmount + pheroAmount, mMaxPheroAmount);
}

bool Script_PheroPlayer::ReducePheroAmount(float pheroCost, bool checkOnly)
{
	if (mCurPheroAmount < pheroCost) {
		return false;
	}

	if (!checkOnly) {
		mCurPheroAmount = max(mCurPheroAmount - pheroCost, 0.f);
	}

	return true;
}
#pragma endregion