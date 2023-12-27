#pragma once
#include "Component.h"

class CameraObject;
class Script_Billboard : public Component {
	COMPONENT(Component, Script_Billboard)

private:
	sptr<CameraObject> mCameraObject;

public:
	virtual void Start() override;
	virtual void Update() override;
};