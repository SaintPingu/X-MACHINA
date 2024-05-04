#pragma once

#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region EnumClass
enum class EnemyState : UINT8 {
	Idle = 0,
	Walk,
	GetHit,
	Attack,
	Death,
};
#pragma endregion


#pragma region ClassForwardDecl
class AnimatorController;
#pragma endregion


#pragma region Class
class Script_EnemyManager : public Component {
	COMPONENT(Script_EnemyManager, Component)

public:
	float mRotationSpeed	= 180.f;
	float mMoveSpeed		= 1.8f;
	float mDetectionRange	= 10.f;
	float mAttackRange		= 2.f;
	float mAttackCoolTime	= 1.5f;
	std::string mAttackName = "Attack";
	EnemyState mState		= EnemyState::Idle;

	sptr<Object> mTarget{};
	std::stack<Vec3> mPath{};
	sptr<AnimatorController> mController{};
};
#pragma endregion
