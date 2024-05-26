#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion

class Script_Item abstract : public Component {
	COMPONENT_ABSTRACT(Script_Item, Component)

public:
	virtual void InteractItem() abstract;
};


class Script_Item_Weapon : public Script_Item {
	COMPONENT_ABSTRACT(Script_Item_Weapon, Script_Item)

public:
	virtual void InteractItem() override;
};