#pragma once

#pragma region Include
#include "Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class CameraObject;
#pragma endregion


#pragma region Class
class Script_Billboard : public Component {
	COMPONENT(Script_Billboard, Component)

private:
	const CameraObject* mCameraObject{};

public:
	virtual void Start() override;
	virtual void Update() override;
};
#pragma endregion