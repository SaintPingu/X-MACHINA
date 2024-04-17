#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class TaskPatrol : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;

	std::vector<Vec3> mWayPoints{};
	int mCurrWayPointIdx{};
	float mPatrolSpeed{};

public:
	TaskPatrol(Object* object, std::vector<Vec3>&& wayPoints);
	virtual ~TaskPatrol() = default;

	virtual BT::NodeState Evaluate() override;
};
#pragma endregion

