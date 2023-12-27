#pragma once
#include "Component.h"

class CGameObject;

class Script_Gunship : public Component {
	COMPONENT(Component, Script_Gunship)

private:
	Transform* mMainRotorFrame{};
	Transform* mTailRotorFrame{};

	CGameObject* mGameObject{};

public:
	virtual void Start() override;
	virtual void Update() override;
};
