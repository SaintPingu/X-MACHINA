#pragma once


#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region Class
// Gunship�� �𵨷� �ϴ� ��ü
// �� ������Ʈ���� ���͸� ȸ���Ѵ�.
class Script_Gunship : public Component {
	COMPONENT(Script_Gunship, Component)

private:
	Transform* mMainRotor{};
	Transform* mTailRotor{};

public:
	virtual void Start() override;
	virtual void Update() override;
};
#pragma endregion