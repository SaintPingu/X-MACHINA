#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class Script_LoginUI;

#pragma region Class
class Script_LoginManager : public Component {
	COMPONENT(Script_LoginManager, Component)

private:
	sptr<Script_LoginUI> mLoginUI{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	void FailLogin();
	static void ChangeToLobbyScene();
};
#pragma endregion