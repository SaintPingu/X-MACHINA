#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
struct AnimatorParameter;

class Script_EnemyManager;
class AnimatorMotion;
#pragma endregion


#pragma region Class
class TaskMoveToPath : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;

	float mMoveSpeed{};
	float mReturnSpeed{};
	std::stack<Vec3>* mPath;

	AnimatorMotion* mWalkMotion{};
	const AnimatorParameter* mReturnParam{};

public:
	TaskMoveToPath(Object* object);
	virtual ~TaskMoveToPath() = default;

public:
	virtual BT::NodeState Evaluate() override;
};
#pragma endregion