#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion


#pragma region Class
class Script_Onyscidus : public Script_Enemy {
	COMPONENT(Script_Onyscidus, Script_Enemy)

public:
	virtual void Awake() override;
	virtual void Update() override;
};

#pragma endregion