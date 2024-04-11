#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#include "TaskPathPlanningAstar.h"
#pragma endregion


#pragma region Using
using namespace Path;
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class TaskMoveToTargetAStar : public TaskPathPlanningAStar {
	using base = TaskPathPlanningAStar;

public:
	TaskMoveToTargetAStar(Object* object);
	virtual ~TaskMoveToTargetAStar() = default;

public:
	virtual BT::NodeState Evaluate() override;
};
#pragma endregion
