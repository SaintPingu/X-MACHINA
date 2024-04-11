#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class TaskMoveToTarget : public BT::Node {
private:
	sptr<Script_EnemyManager>	mEnemyMgr;
	sptr<GridObject>			mGridTarget;

public:
	TaskMoveToTarget(Object* object);
	virtual ~TaskMoveToTarget() = default;

public:
	virtual BT::NodeState Evaluate() override;
	
	bool PathPlanningAStar(const Vec3& targetPos);
};
#pragma endregion
