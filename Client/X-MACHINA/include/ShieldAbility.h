#pragma once

#include "AbilityMgr.h"

class Script_LiveObject;
class GameObject;

class ShieldAbility : public RenderedAbility {
	using base = RenderedAbility;

private:
	float mShield{};
	float mPheroCost{};

public:
	ShieldAbility(float sheild);

public:
	virtual void Update(float activeTime) override;
	virtual bool Activate() override;
	virtual void DeActivate() override;
};
