#pragma once

#include "AbilityMgr.h"

class GameObject;
class UI;

class MinimapAbility : public RenderedAbility {
	using base = RenderedAbility;

private:
	sptr<UI> mUI{};
	sptr<UI> mPlayerUI{};

public:
	MinimapAbility();

public:
	virtual void Update(float activeTime) override;
	virtual bool Activate() override;
	virtual void DeActivate() override;
	virtual void Render() override;
};
