#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region Class
class WeaponUI;
class Script_Weapon;

class Script_BattleUI : public Component {
	COMPONENT(Script_BattleUI, Component)

private:
	std::vector<sptr<WeaponUI>> mWeaponUIs{};

public:
	virtual void Awake() override;

public:
	void UpdateWeapon(int idx) const;
	void SetWeapon(int idx, rsptr<Script_Weapon> weapon) const;
	int CreateWeaponUI();
};

#pragma endregion