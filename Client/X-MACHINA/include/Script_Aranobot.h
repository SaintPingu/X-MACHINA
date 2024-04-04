#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class AnimatorController;
#pragma endregion


#pragma region Class
class Script_Aranobot : public Script_Enemy {
	COMPONENT(Script_Aranobot, Script_Enemy)

public:
	virtual void Awake() override;
	virtual void Update() override;
};

#pragma endregion