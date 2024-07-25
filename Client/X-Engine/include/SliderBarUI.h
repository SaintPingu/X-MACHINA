#pragma once


#pragma region ClassForwardDecl
class SliderUI;
class UI;
#pragma endregion

// �����̴��� UI
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
	// ���� ���ҵǱ� �� ������ �ش� �Լ��� ȣ������� �Ѵ�. 
	void Decrease(float originalValue) { mDisplayEaseValue = originalValue; }

public:
	void Update(float currValue);

private:
	void AddDisplayValue(float currValue);
	void ReduceDisplayValue(float currValue);
};