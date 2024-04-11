#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class CheckPatrolRange : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;
	
	Vec3 mSpawnPos{};

public:
	CheckPatrolRange(Object* object);
	virtual ~CheckPatrolRange() = default;

	virtual BT::NodeState Evaluate() override;
};
#pragma endregion
