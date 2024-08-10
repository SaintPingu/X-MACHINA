#pragma once


#pragma region ClassForwardDecl
class SliderUI;
class UI;
#pragma endregion


class CooldownCircleUI {
private:
	SliderUI* mCooldownBarUI;

public:
	CooldownCircleUI(const std::string& cooldownBarUI, const Vec2& pos, const Vec2& size, float maxValue);
	virtual ~CooldownCircleUI();

public:
	void On();
	void Off();
	void Update(float currValue, const Vec2& pos);
};

