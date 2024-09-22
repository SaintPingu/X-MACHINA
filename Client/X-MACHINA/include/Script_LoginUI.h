#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class PopupUI;
class Button;
class InputField;
;
#pragma region Class
class Script_LoginUI : public Component {
	COMPONENT(Script_LoginUI, Component)

private:
	sptr<PopupUI> mLoginFailPopup{};

	Button* mLoginButton{};
	InputField* mInput_ID{};
	InputField* mInput_PW{};

public:
	virtual void Awake() override;

public:
	void FailLogin();

private:
	void SendLoginPacket();
	void CloseLoginFailPopupCallback();
};

#pragma endregion