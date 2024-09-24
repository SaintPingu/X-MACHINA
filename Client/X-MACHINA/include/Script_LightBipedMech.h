#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion

class Script_Weapon_LightBiped;

#pragma region Class
class Script_LightBipedMech : public Script_Enemy {
	COMPONENT(Script_LightBipedMech, Script_Enemy)

	Object* mWeapon{};
	sptr<Script_Weapon_LightBiped> mWeaponScript{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void StartAttack() override;
	virtual void OnDestroy() override;
};
#pragma endregion

