#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

#pragma region Class
class Script_LobbyUI : public Component {
	COMPONENT(Script_LobbyUI, Component)

public:
	virtual void Start() override;
	virtual void Update() override;
};

#pragma endregion