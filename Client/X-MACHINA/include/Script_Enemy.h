#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_LiveObject.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class AnimatorController;
#pragma endregion


#pragma region Class
class Script_Enemy abstract : public Script_LiveObject {
	COMPONENT_ABSTRACT(Script_Enemy, Script_LiveObject)

protected:
	float mRotationSpeed  = 270.f;
	float mMoveSpeed      = 1.8f;
	float mDetectionRange = 10.f;

	sptr<AnimatorController>   mController{};

	sptr<GridObject> mPlayer{};

public:
	virtual void Awake() override;
};

#pragma endregion