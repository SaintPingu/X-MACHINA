#pragma once
#include "Component.h"

//-----------------------------[Class Declaration]-----------------------------//
class CameraObject;
//-----------------------------------------------------------------------------//

class Script_Billboard : public Component {
	COMPONENT(Component, Script_Billboard)

private:
	const CameraObject* mCameraObject;

public:
	virtual void Start() override;
	virtual void Update() override;
};