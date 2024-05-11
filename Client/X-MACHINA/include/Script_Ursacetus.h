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
class Script_Ursacetus : public Script_Enemy {
	COMPONENT(Script_Ursacetus, Script_Enemy)

public:
	virtual void Awake() override;
	virtual void Update() override;

	virtual void Attack() override;
};

#pragma endregion