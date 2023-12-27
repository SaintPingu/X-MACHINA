#pragma once
#include "Component.h"

class GameObject;

class Script_Apache : public Component {
	COMPONENT(Component, Script_Apache)

private:
	Transform* mMainRotorFrame{};
	Transform* mTailRotorFrame{};

	GameObject* mGameObject{};

public:
	virtual void Start() override;
	virtual void Update() override;
};
