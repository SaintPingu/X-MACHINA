#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class UI;
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

	UI* mBackground{};
	UI* mBackgroundAfter{};

	static constexpr float mkMaxChangeBackgroundDelay{ 5.f };
	float mCurChangeBackgroundDelay{};

	int mCurBackgroundIdx;
	float mFade_t{};
	float mFadeSpeed{};

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	void FailLogin();

private:
	void SendLoginPacket();
	void CloseLoginFailPopupCallback();

	std::string GetNextBackgroundIdx();
};

#pragma endregion