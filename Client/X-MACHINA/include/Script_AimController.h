#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion

#pragma region ClassForwardDecl
class UI;
#pragma endregion

#pragma region Class
class Script_AimController : public Component {
	COMPONENT(Script_AimController, Component)

private:
	sptr<UI> mUI;
	Vec2 mMousePos{};

	float mMaxXPos;
	float mMaxYPos;

	float mouseSensitivity{ 2.f };

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	Vec3 GetAimDirection() const;
};
#pragma endregion