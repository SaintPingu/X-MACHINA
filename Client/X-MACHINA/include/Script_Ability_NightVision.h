#pragma once


#pragma region Include
#include "Script_Ability.h"
#include "PheroAbilityInterface.h"
#pragma endregion


#pragma region Class
class Script_Ability_NightVision : public Script_RenderedAbility {
	COMPONENT(Script_Ability_NightVision, Script_RenderedAbility)

public:
	virtual void Awake() override;

public:
	virtual void On() override;
	virtual void Off() override;

};
#pragma endregion

