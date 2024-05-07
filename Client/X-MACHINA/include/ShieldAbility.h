#pragma once

#include "AbilityMgr.h"

class GameObject;

class ShieldAbility : public RenderedAbility {
public:
	ShieldAbility(float cooldownTime, float activeTime);

public:
	virtual void Update() override;
};
