#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region Class
// Apache�� �𵨷� �ϴ� ��ü
// �� ������Ʈ���� ���͸� ȸ���Ѵ�.
class Script_Apache : public Component {
	COMPONENT(Script_Apache, Component)

private:
	Transform* mMainRotor{};
	Transform* mTailRotor{};

public:
	virtual void Start() override;
	virtual void Update() override;
};
#pragma endregion