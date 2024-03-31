#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
#pragma endregion


#pragma region Class
class Script_GroundObject : public Component {
	COMPONENT(Script_GroundObject, Component)

public:
	virtual void LateUpdate() override;
};

#pragma endregion