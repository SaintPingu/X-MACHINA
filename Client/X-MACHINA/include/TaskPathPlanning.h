#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region Using
using namespace Path;
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
#pragma endregion


#pragma region Struct
struct PQNode {
	bool operator<(const PQNode& rhs) const { return F < rhs.F; }
	bool operator>(const PQNode& rhs) const { return F > rhs.F; }

	int	F;
	int	G;
	Pos	Pos;
};
#pragma endregion


#pragma region Class
class TaskPathPlanning : public BT::Node {
private:
	sptr<Script_EnemyManager>	mEnemyMgr;

	std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;
	std::stack<Vec3>		mPath{};

	static constexpr int	mkWeight = 10;
	static constexpr int	mkMaxVisited = 2000;
public:
	TaskPathPlanning(Object* object);
	virtual ~TaskPathPlanning() = default;

public:
	virtual BT::NodeState Evaluate() override;
	bool PathPlanningAStar(Pos start, Pos dest);
};
#pragma endregion
