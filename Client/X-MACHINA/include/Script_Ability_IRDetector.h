#pragma once


#pragma region Include
#include "Script_Ability.h"
#include "PheroAbilityInterface.h"
#pragma endregion


#pragma region Class
class Script_Ability_IRDetector : public Script_RenderedAbility {
	COMPONENT(Script_Ability_IRDetector, Script_RenderedAbility)

public:
	virtual void Awake() override;

public:
	virtual void On() override;
	virtual void Off() override;

};
#pragma endregion

