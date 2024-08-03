#pragma once

#include "Script_Ability.h"

class GameObject;

#pragma region Script_Ability_Indicator
class Script_Ability_AttackIndicator : public Script_RenderedAbility {
	COMPONENT(Script_Ability_AttackIndicator, Script_RenderedAbility)

public:
	virtual void Init(float activeTime, const std::string& indicatorShape);
	virtual void On() override;
};
#pragma endregion