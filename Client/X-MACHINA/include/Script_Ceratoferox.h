#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion


#pragma region Class
class Script_Ceratoferox : public Script_Enemy {
	COMPONENT(Script_Ceratoferox, Script_Enemy)

public:
	virtual void Awake() override;
};

#pragma endregion