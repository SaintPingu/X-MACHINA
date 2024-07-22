#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion


#pragma region Class
class Script_LightBipedMech : public Script_Enemy {
	COMPONENT(Script_LightBipedMech, Script_Enemy)

	Transform* mWeapon{};
public:
	virtual void Awake() override;
	virtual void OnDestroy() override;
};
#pragma endregion

