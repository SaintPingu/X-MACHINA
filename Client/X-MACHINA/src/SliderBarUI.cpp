#include "stdafx.h"
#include "SliderBarUI.h"

#include "Component/UI.h"





#pragma region MainPlayer
SliderBarUI::SliderBarUI(const std::string& backgroundTexture, const std::string& easeTexture, const std::string& fillTexture, const Vec2& pos, const Vec2& size, float maxValue)
{
	mBackgroundBarUI = Canvas::I->CreateUI<UI>(0, backgroundTexture, pos, size);

	mEaseBarUI = Canvas::I->CreateUI<SliderUI>(1, easeTexture, pos, size);
	mEaseBarUI->SetMinMaxValue(0.f, maxValue);

	mFillBarUI = Canvas::I->CreateUI<SliderUI>(2, fillTexture, pos, size);
	mFillBarUI->SetMinMaxValue(0.f, maxValue);
}

void SliderBarUI::Update(float currPheroAmount)
{
	if (mDisplayFillValue < currPheroAmount) {
		AddDisplayValue(currPheroAmount);
		mFillBarUI->SetValue(mDisplayFillValue);
	}
	if (mDisplayEaseValue > currPheroAmount) {
		ReduceDisplayValue(currPheroAmount);
		mEaseBarUI->SetValue(mDisplayEaseValue);
		mFillBarUI->SetValue(mDisplayFillValue);
	}
}

void SliderBarUI::AddDisplayValue(float currPheroAmount)
{
	constexpr float lerpSpeed = 0.01f;

	// ������ ������ ���� �ε巴�� ��� ����
	mDisplayFillValue = Math::LerpFloat(mDisplayFillValue, currPheroAmount, lerpSpeed);
	if ((currPheroAmount - mDisplayFillValue) <= 0.01f) {
		mDisplayFillValue = currPheroAmount;
	}
}

void SliderBarUI::ReduceDisplayValue(float currPheroAmount)
{
	constexpr float lerpSpeed = 0.05f;

	// ���ҽ� �ٷ� ��� ����
	mDisplayFillValue = currPheroAmount;

	mDisplayEaseValue = Math::LerpFloat(mDisplayEaseValue, currPheroAmount, lerpSpeed);
	if ((mDisplayEaseValue - currPheroAmount) <= 0.01f) {
		mDisplayEaseValue = currPheroAmount;
	}
}
#pragma endregion