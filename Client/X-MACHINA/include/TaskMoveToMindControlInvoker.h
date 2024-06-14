#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class TaskMoveToMindControlInvoker : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;
	Object* mInvoker;

public:
	TaskMoveToMindControlInvoker(Object* object, Object* invoker);
	virtual ~TaskMoveToMindControlInvoker() = default;

public:
	virtual BT::NodeState Evaluate() override;
};
#pragma endregion
