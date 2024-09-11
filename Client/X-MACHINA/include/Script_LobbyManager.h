#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

#pragma region Class
class Script_LobbyManager : public Component {
	COMPONENT(Script_LobbyManager, Component)

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	static void ChangeToBattleScene();
};

#pragma endregion