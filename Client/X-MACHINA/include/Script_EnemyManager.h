#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class AnimatorController;
#pragma endregion


#pragma region Class
class Script_EnemyManager : public Component {
	COMPONENT(Script_EnemyManager, Component)

public:
	float mRotationSpeed = 270.f;
	float mMoveSpeed = 1.8f;
	float mDetectionRange = 10.f;
	float mAttackRange = 2.f;
	float mAttackCoolTime = 1.5f;

	std::stack<Vec3> mPath{};

	sptr<AnimatorController>   mController{};
};
#pragma endregion
