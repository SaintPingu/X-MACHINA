#include "stdafx.h"
#include "Script_Player.h"

#include "SliderBarUI.h"
#include "Timer.h"





#pragma region Player
void Script_PheroPlayer::Start()
{
	base::Start();

	mStartPheroAmount = 100.f;
	mCurrPheroAmount = 100.f;
	mMaxPheroAmount = 1000.f;
	mPheroRegenRate = 3.f; // 초당 페로 회복량 (초당 3회복)

	mPheroBarUI = std::make_shared<SliderBarUI>("BackgroundPheroBar", "EaseBar", "FillPheroBar", Vec2{0.f, -900.f}, Vec2{1000.f, 15.f}, mMaxPheroAmount);
}

void Script_PheroPlayer::Update()
{
	base::Update();

	AddPheroAmount(DeltaTime() * mPheroRegenRate);
	mPheroBarUI->Update(mCurrPheroAmount);
}

void Script_PheroPlayer::Respawn()
{
	Script_Player::Respawn();

	mCurrPheroAmount = mStartPheroAmount;
}

void Script_PheroPlayer::AddPheroAmount(float pheroAmount)
{
	mCurrPheroAmount = min(mCurrPheroAmount + pheroAmount, mMaxPheroAmount);
}

bool Script_PheroPlayer::ReducePheroAmount(float pheroCost)
{
	if (mCurrPheroAmount < pheroCost) {
		return false;
	}

	mPheroBarUI->MustCallBeforeOnValueDecrease(mCurrPheroAmount);

	mCurrPheroAmount = max(mCurrPheroAmount - pheroCost, 0.f);
	return true;
}
#pragma endregion