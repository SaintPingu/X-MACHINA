#pragma once
#include "Component.h"

class CGameObject;

class Script_Apache : public Component {
	COMPONENT(Component, Script_Apache)

private:
	Transform* mMainRotorFrame{};
	Transform* mTailRotorFrame{};

	CGameObject* mGameObject{};

public:
	virtual void Start() override;
	virtual void Update() override;
};
