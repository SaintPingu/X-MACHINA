#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
// Gunship을 모델로 하는 객체
// 매 업데이트마다 로터를 회전한다.
class Script_Gunship : public Component {
	COMPONENT(Script_Gunship, Component)

private:
	Transform* mMainRotor{};
	Transform* mTailRotor{};

	GameObject* mGameObject{};

public:
	virtual void Start() override;
	virtual void Update() override;
};
#pragma endregion