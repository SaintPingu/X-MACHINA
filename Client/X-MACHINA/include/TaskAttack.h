#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Script_EnemyManager;
class Script_LiveObject;
#pragma endregion


#pragma region Class
class TaskAttack : public BT::ActionNode {
private:
	sptr<Script_EnemyManager> mEnemyMgr;
	sptr<Script_LiveObject> mLiveObject;

public:
	TaskAttack(Object* object, std::function<void()> callback = nullptr);
	virtual ~TaskAttack() = default;

public:
	virtual BT::NodeState Evaluate() override;

};
#pragma endregion
