#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion

#pragma region ClassForwardDecl
class UI;
#pragma endregion

#pragma region Class
class Script_AimUI : public Component {
	COMPONENT(Script_AimUI, Component)

private:
	sptr<UI> mUI;
	Vec2 mMousePos{};

public:
	virtual void Awake() override;
};
#pragma endregion