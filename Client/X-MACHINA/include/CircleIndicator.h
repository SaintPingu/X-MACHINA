#pragma once

#include "AbilityMgr.h"

class GameObject;

class AttackIndicator : public RenderedAbility {
	using base = RenderedAbility;

public:
	AttackIndicator(float activeTime, std::string indicatorShape);

public:
	virtual void Update(float activeTime) override;
	virtual void Activate() override;
	virtual void DeActivate() override;
};