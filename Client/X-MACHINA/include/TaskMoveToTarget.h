#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Script_EnemyManager;
#pragma endregion


#pragma region ClassForwardDecl
struct PQNode {
	bool operator<(const PQNode& rhs) const { return F < rhs.F; }
	bool operator>(const PQNode& rhs) const { return F > rhs.F; }

	INT32	F; // f = g + h
	INT32	G;
	Pos		Pos;
};
#pragma endregion


#pragma region Class
class TaskMoveToTarget : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;

	float mAStarAcctime{};
	std::stack<Vec3> mPath{};

public:
	TaskMoveToTarget(Object* object);
	virtual ~TaskMoveToTarget() = default;

public:
	virtual BT::NodeState Evaluate() override;
	
	void PathPlanningAStar(const Vec3& targetPos);
};
#pragma endregion
