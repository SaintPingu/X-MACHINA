#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
class Script_Gunship : public Component {
	COMPONENT(Script_Gunship, Component)

private:
	Transform* mMainRotorFrame{};
	Transform* mTailRotorFrame{};

	GameObject* mGameObject{};

public:
	virtual void Start() override;
	virtual void Update() override;
};
#pragma endregion