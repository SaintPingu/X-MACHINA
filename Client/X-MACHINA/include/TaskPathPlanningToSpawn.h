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
class TaskPathPlanningToSpawn : public TaskPathPlanningAStar {
	using base = TaskPathPlanningAStar;

private:
	Vec3 mSpawnPos{};

public:
	TaskPathPlanningToSpawn(Object* object);
	virtual ~TaskPathPlanningToSpawn() = default;

public:
	virtual BT::NodeState Evaluate() override;
};
#pragma endregion
