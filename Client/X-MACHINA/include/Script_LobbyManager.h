#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion

class UI;
struct UITexture;
class Script_AimController;

#pragma region Class
class Script_LobbyManager : public Component {
	COMPONENT(Script_LobbyManager, Component)

private:
	sptr<Script_AimController> mAimController{};
	UI* mCursorNormal{};
	UI* mCursorClick{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void Reset() override;

public:
	void ChangeToBattleScene();
};

#pragma endregion