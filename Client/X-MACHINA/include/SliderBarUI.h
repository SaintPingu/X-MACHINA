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
	float mCurValue{};

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
	virtual ~SliderBarUI();

public:
	void SetDissolve(const Vec3& color);

public:
	void Update(float currValue);

private:
	void AddDisplayValue(float currValue);
	void ReduceDisplayValue(float currValue);
};