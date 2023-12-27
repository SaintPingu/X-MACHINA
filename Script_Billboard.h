#pragma once
#include "Component.h"

class CCameraObject;
class Script_Billboard : public Component {
	COMPONENT(Component, Script_Billboard)

private:
	sptr<CCameraObject> mCameraObject;

public:
	virtual void Start() override;
	virtual void Update() override;
};