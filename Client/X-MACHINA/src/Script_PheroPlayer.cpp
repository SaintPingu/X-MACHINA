#include "stdafx.h"
#include "Script_Player.h"

void Script_PheroPlayer::Start()
{
	base::Start();

	mPheroAmount = 0.f;
	mMaxPheroAmount = 1000.f;
}

void Script_PheroPlayer::AddPheroAmount(float pheroAmount)
{
	mPheroAmount = min(mPheroAmount + pheroAmount, mMaxPheroAmount);
}

void Script_PheroPlayer::ReducePheroAmount(float pheroAmount)
{
	mPheroAmount = max(mPheroAmount - pheroAmount, 0.f);
}
