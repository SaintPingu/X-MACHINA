#pragma once

#include "AbilityMgr.h"

class GameObject;

class CircleIndicator : public RenderedAbility {
	using base = RenderedAbility;

public:
	CircleIndicator(float activeTime);

public:
	virtual void Update(float activeTime) override;
	virtual void Activate() override;
	virtual void DeActivate() override;
};
