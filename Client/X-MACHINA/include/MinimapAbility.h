#pragma once

#include "AbilityMgr.h"

class GameObject;
class UI;

class MinimapAbility : public RenderedAbility {
	using base = RenderedAbility;

private:
	UI* mUI{};
	UI* mPlayerUI{};

public:
	MinimapAbility();

public:
	virtual void Update(float activeTime) override;
	virtual void Activate() override;
	virtual void DeActivate() override;
	virtual void Render() override;
};
