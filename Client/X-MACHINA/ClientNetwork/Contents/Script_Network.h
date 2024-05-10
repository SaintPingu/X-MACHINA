#pragma once
#pragma region Include
#include "Component/Component.h"
#pragma endregion

#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


class Script_Network abstract : public Component {
	COMPONENT_ABSTRACT(Script_Network, Component)

public:
	virtual void UpdateData(const void* data) abstract;
};
