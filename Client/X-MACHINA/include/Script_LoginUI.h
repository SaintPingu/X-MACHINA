#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class UI;
class PopupUI;
class Script_AimController;

#pragma region Class
class Script_LoginUI : public Component {
	COMPONENT(Script_LoginUI, Component)

public:
	virtual void Awake() override;
};

#pragma endregion