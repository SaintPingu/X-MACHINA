#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class InstObject;
#pragma endregion


#pragma region Class
class Script_Phero : public Component {
	COMPONENT(Script_Phero, Component)

private:
	InstObject* mGameObject{};
};
#pragma endregion

