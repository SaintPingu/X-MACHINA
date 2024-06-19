#pragma once

class PheroAbilityInterface {
protected:
	float mPheroCost{};

public:
	PheroAbilityInterface() = delete;
	PheroAbilityInterface(float cost) : mPheroCost(cost) {}
	virtual ~PheroAbilityInterface() = default;

protected:
	virtual bool ReducePheroAmount(bool checkOnly = false) abstract;
};

