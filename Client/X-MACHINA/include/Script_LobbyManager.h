#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class UI;

#pragma region Class
class Script_LobbyManager : public Component {
	COMPONENT(Script_LobbyManager, Component)

private:
	UI* mCursor{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void Reset() override;

public:
	void Test();
};

#pragma endregion