#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class CheckAttackRange : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;

public:
	CheckAttackRange(Object* object);
	virtual ~CheckAttackRange() = default;

	virtual BT::NodeState Evaluate() override;
};
#pragma endregion


