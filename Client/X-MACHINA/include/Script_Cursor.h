#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class UI;
class PopupUI;
class Script_AimController;

#pragma region Class
class Script_Cursor : public Component {
	COMPONENT(Script_Cursor, Component)

private:
	sptr<Script_AimController> mAimController{};
	UI* mCursorNormal{};
	UI* mCursorClick{};

public:
	virtual void Awake() override;
	virtual void Update() override;
};

#pragma endregion