#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
#pragma endregion


#pragma region Class
// Gunship�� �𵨷� �ϴ� ��ü
// �� ������Ʈ���� ���͸� ȸ���Ѵ�.
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