#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion

class Script_Item abstract : public Component {
	COMPONENT_ABSTRACT(Script_Item, Component)

public:
	virtual void Interact() abstract;
};


class Script_Item_WeaponCrate : public Script_Item , SceneScript {
	COMPONENT_ABSTRACT(Script_Item_WeaponCrate, Script_Item)

public:
	std::string type;
	int count;
	float hp;
	bool can_open;

public:
	virtual void Awake() override;

public:
	virtual void LoadData(rsptr<ScriptExporter> exporter) override;
	virtual void Interact() override;
};