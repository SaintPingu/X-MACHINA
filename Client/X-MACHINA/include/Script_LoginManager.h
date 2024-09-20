#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region Class
class Script_LoginManager : public Component {
	COMPONENT(Script_LoginManager, Component)

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	static void ChangeToLobbyScene();
};
#pragma endregion