#pragma once

#include "AbilityMgr.h"
#include "PheroAbilityInterface.h"

class Script_LiveObject;
class GameObject;

class ShieldAbility : public RenderedAbility, public PheroAbilityInterface {
	using base = RenderedAbility;
private:
	float mShieldAmount{};

public:
	ShieldAbility(float sheild);

public:
	virtual void Update(float activeTime) override;
	virtual bool Activate() override;
	virtual void DeActivate() override;

protected:
	virtual bool ReducePheroAmount() override;
};
