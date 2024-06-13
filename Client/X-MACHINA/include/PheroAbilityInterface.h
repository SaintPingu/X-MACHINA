#pragma once

class PheroAbilityInterface {
protected:
	float mPheroCost{};

public:
	PheroAbilityInterface(float cost) : mPheroCost(cost) {}

protected:
	virtual bool ReducePheroAmount() abstract;
};

