#include "EnginePch.h"
#include "SliderBarUI.h"

#include "Component/UI.h"





#pragma region MainPlayer
SliderBarUI::SliderBarUI(const std::string& backgroundTexture, const std::string& easeTexture, const std::string& fillTexture, const Vec2& pos, const Vec2& size, float maxValue)
{
	mBackgroundBarUI = Canvas::I->CreateUI(0, backgroundTexture, pos, size.x, size.y);

	mEaseBarUI = Canvas::I->CreateSliderUI(1, easeTexture, pos, size.x, size.y);
	mEaseBarUI->SetMinMaxValue(0.f, maxValue);

	mFillBarUI = Canvas::I->CreateSliderUI(2, fillTexture, pos, size.x, size.y);
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

	// 증가시 보간을 통해 부드럽게 페로 증가
	mDisplayFillValue = Math::LerpFloat(mDisplayFillValue, currPheroAmount, lerpSpeed);
	if ((currPheroAmount - mDisplayFillValue) <= 0.01f) {
		mDisplayFillValue = currPheroAmount;
	}
}

void SliderBarUI::ReduceDisplayValue(float currPheroAmount)
{
	constexpr float lerpSpeed = 0.05f;

	// 감소시 바로 페로 감소
	mDisplayFillValue = currPheroAmount;

	mDisplayEaseValue = Math::LerpFloat(mDisplayEaseValue, currPheroAmount, lerpSpeed);
	if ((mDisplayEaseValue - currPheroAmount) <= 0.01f) {
		mDisplayEaseValue = currPheroAmount;
	}
}
#pragma endregion