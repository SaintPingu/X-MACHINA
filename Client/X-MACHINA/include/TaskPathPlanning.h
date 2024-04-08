#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
#pragma endregion


#pragma region Struct
struct PQNode {
	bool operator<(const PQNode& rhs) const { return F < rhs.F; }
	bool operator>(const PQNode& rhs) const { return F > rhs.F; }

	INT32	F;
	INT32	G;
	Pos		Pos;
};
#pragma endregion


#pragma region Class
class TaskPathPlanning : public BT::Node {
private:
	std::stack<Vec3>		mPath{};
	std::map<Pos, Pos>		mParent;
	std::map<Pos, INT32>	mDistance;
	std::map<Pos, bool>		mVisited;
	std::priority_queue<PQNode, std::vector<PQNode>, std::greater<PQNode>> pq;
	static constexpr int	mkWeight = 10;
	static constexpr int	mkMaxVisited = 1000;
	static constexpr int	mkPathAdjust = 3;

	float mAccTime = 0.f;

	sptr<Script_EnemyManager>	mEnemyMgr;

public:
	TaskPathPlanning(Object* object);
	virtual ~TaskPathPlanning() = default;

public:
	virtual BT::NodeState Evaluate() override;
	bool PathPlanningAStar(Pos start, Pos dest);
};
#pragma endregion
