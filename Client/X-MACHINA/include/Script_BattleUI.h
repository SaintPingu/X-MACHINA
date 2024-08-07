#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region Class
class WeaponUI;

class Script_BattleUI : public Component {
	COMPONENT(Script_BattleUI, Component)

private:
	std::vector<sptr<WeaponUI>> mWeaponUIs{};

public:
	virtual void Awake() override;
	virtual void Update() override;
};

#pragma endregion