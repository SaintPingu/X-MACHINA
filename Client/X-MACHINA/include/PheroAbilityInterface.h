#pragma once

#pragma region ClassForwardDecl
class CooldownCircleUI;
class SliderUI;
class UI;
#pragma endregion

class PheroAbilityInterface {
protected:
	float mPheroCost{};

protected:
	void SetPheroCost(float cost) { mPheroCost = cost; }
	virtual bool ReducePheroAmount(bool checkOnly = false) abstract;
};

class IconAbilityInterface {
protected:
	UI* mAbilityIconUI{};
	SliderUI* mCooldownUI{};

protected:
	void SetIconUI(const std::string& background, const std::string& cooldown, const Vec2& pos, const Vec2& size, float maxValue);
	void UpdateCooldownBarUI(float maxValue, float currValue);
};
