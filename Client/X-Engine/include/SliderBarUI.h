#pragma once


#pragma region ClassForwardDecl
class SliderUI;
class UI;
#pragma endregion

// 슬라이더바 UI
class SliderBarUI {
private:
	float mDisplayFillValue{};
	float mDisplayEaseValue{};

	SliderUI* mFillBarUI;
	SliderUI* mEaseBarUI;
	UI* mBackgroundBarUI;

public:
	SliderBarUI(
		const std::string& backgroundTexture,
		const std::string& easeTexture,
		const std::string& fillTexture,
		const Vec2& pos,
		const Vec2& size,
		float maxValue
	);

public:
	// 값이 감소되기 전 무조건 해당 함수를 호출해줘야 한다. 
	void Decrease(float originalValue) { mDisplayEaseValue = originalValue; }

public:
	void Update(float currValue);

private:
	void AddDisplayValue(float currValue);
	void ReduceDisplayValue(float currValue);
};