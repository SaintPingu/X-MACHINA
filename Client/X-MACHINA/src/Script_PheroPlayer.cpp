#include "stdafx.h"
#include "Script_Player.h"

#include "Component/UI.h"
#include "Timer.h"


#pragma region Player
void Script_PheroPlayer::Start()
{
	base::Start();

	mCurrPheroAmount = 100.f;
	mMaxPheroAmount = 1000.f;
}

void Script_PheroPlayer::Update()
{
	base::Update();
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

	mCurrPheroAmount = max(mCurrPheroAmount - pheroCost, 0.f);
	return true;
}
#pragma endregion





#pragma region MainPlayer
void Script_PheroMainPlayer::Start()
{
	base::Start();

	constexpr Vec2 posUI = Vec2{ 0.f, -900.f };
	constexpr Vec2 sizeUI = Vec2{ 1000.f, 15.f };

	mBackgroundPheroBarUI = Canvas::I->CreateUI(0, "BackgroundPheroBar", posUI, sizeUI.x, sizeUI.y);

	mEasePheroBarUI = Canvas::I->CreateSliderUI(1, "EasePheroBar", posUI, sizeUI.x, sizeUI.y);
	mEasePheroBarUI->SetMinMaxValue(0.f, mMaxPheroAmount);

	mFillPheroBarUI = Canvas::I->CreateSliderUI(2, "FillPheroBar", posUI, sizeUI.x, sizeUI.y);
	mFillPheroBarUI->SetMinMaxValue(0.f, mMaxPheroAmount);
}

void Script_PheroMainPlayer::Update()
{
	base::Update();

	if (mDisplayFillPheroAmount < mCurrPheroAmount) {
		AddDisplayPheroAmount();
		mFillPheroBarUI->SetValue(mDisplayFillPheroAmount);
	}
	if (mDisplayEasePheroAmount > mCurrPheroAmount) {
		ReduceDisplayPheroAmount();
		mEasePheroBarUI->SetValue(mDisplayEasePheroAmount);
		mFillPheroBarUI->SetValue(mDisplayFillPheroAmount);
	}
}

void Script_PheroMainPlayer::AddPheroAmount(float pheroAmount)
{
	base::AddPheroAmount(pheroAmount);
}

bool Script_PheroMainPlayer::ReducePheroAmount(float pheroCost)
{
	if (mCurrPheroAmount < pheroCost) {
		return false;
	}

	mDisplayEasePheroAmount = mCurrPheroAmount;
	mCurrPheroAmount = max(mCurrPheroAmount - pheroCost, 0.f);
	return true;
}

void Script_PheroMainPlayer::AddDisplayPheroAmount()
{
	constexpr float lerpSpeed = 0.01f;

	// 증가시 보간을 통해 부드럽게 페로 증가
	mDisplayFillPheroAmount = Math::LerpFloat(mDisplayFillPheroAmount, mCurrPheroAmount, lerpSpeed);
	if ((mCurrPheroAmount - mDisplayFillPheroAmount) <= 0.01f) {
		mDisplayFillPheroAmount = mCurrPheroAmount;
	}
}

void Script_PheroMainPlayer::ReduceDisplayPheroAmount()
{
	constexpr float lerpSpeed = 0.05f;

	// 감소시 바로 페로 감소
	mDisplayFillPheroAmount = mCurrPheroAmount;

	mDisplayEasePheroAmount = Math::LerpFloat(mDisplayEasePheroAmount, mCurrPheroAmount, lerpSpeed);
	if ((mDisplayEasePheroAmount - mCurrPheroAmount) <= 0.01f) {
		mDisplayEasePheroAmount = mCurrPheroAmount;
	}
}
#pragma endregion
