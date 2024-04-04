#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class TaskAttack : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;
	float mAttackAccTime = 0.f;

public:
	TaskAttack(Object* object);
	virtual ~TaskAttack() = default;

	virtual BT::NodeState Evaluate() override;
};
#pragma endregion
