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
class Script_Enemy : public Script_LiveObject {
	COMPONENT(Script_Enemy, Script_LiveObject)

private:
	const float mkRotationSpeed = 270.f;
	float mMoveSpeed            = 1.8f;
	float mDetectionRange       = 10.f;

	sptr<AnimatorController>   mController{};

	sptr<GridObject> mGun{};
	sptr<GridObject> mPlayer{};
	sptr<GridObject> mTarget{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

	virtual void OnDestroy() override;
};

#pragma endregion