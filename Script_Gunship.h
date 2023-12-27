#pragma once
#include "Component.h"

class GameObject;

class Script_Gunship : public Component {
	COMPONENT(Component, Script_Gunship)

private:
	Transform* mMainRotorFrame{};
	Transform* mTailRotorFrame{};

	GameObject* mGameObject{};

public:
	virtual void Start() override;
	virtual void Update() override;
};
