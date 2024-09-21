#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class UI;
class PopupUI;
class Button;
class InputField;
class Script_AimController;

#pragma region Class
class Script_LoginUI : public Component {
	COMPONENT(Script_LoginUI, Component)

private:
	Button* mLoginButton{};
	InputField* mInput_ID{};
	InputField* mInput_PW{};

public:
	virtual void Awake() override;

private:
	void SendLoginPacket();
};

#pragma endregion