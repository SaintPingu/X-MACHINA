#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class ObjectPool;
#pragma endregion


#pragma region Class
class Script_PheroObject : public Component {
	COMPONENT(Script_PheroObject, Component)

public:
	virtual void Start() override;
	virtual void OnDestroy() override;

};
#pragma endregion
