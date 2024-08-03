#pragma once

class PheroAbilityInterface {
protected:
	float mPheroCost{};

protected:
	void SetPheroCost(float cost) { mPheroCost = cost; }
	virtual bool ReducePheroAmount(bool checkOnly = false) abstract;
};

