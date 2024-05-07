#pragma once

#include "AbilityMgr.h"

class GameObject;

class ShieldAbility : public RenderedAbility {
	using base = RenderedAbility;

public:
	ShieldAbility(float cooldownTime, float activeTime);

public:
	virtual void Update() override;
	virtual void Activate() override;

private:
	void FollowObject();
};
