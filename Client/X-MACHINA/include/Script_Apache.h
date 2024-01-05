#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region Class
// Apache를 모델로 하는 객체
// 매 업데이트마다 로터를 회전한다.
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