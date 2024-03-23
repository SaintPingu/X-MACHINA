#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region Class
// 매 업데이트마다 카메라를 향해 회전한다.
class Script_Billboard : public Component {
	COMPONENT(Script_Billboard, Component)

protected:
	float mScale{ 1.f };

public:
	void SetScale(float scale) { mScale = scale; }

public:
	virtual void Update() override;

public:
	virtual void UpdateSpriteVariable(const int matIndex) const;
};
#pragma endregion