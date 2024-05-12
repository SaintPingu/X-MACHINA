#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion


#pragma region Class
class Script_AdvancedCombatDroid_5 : public Script_Enemy {
	COMPONENT(Script_AdvancedCombatDroid_5, Script_Enemy)

public:
	virtual void Awake() override;
};

#pragma endregion