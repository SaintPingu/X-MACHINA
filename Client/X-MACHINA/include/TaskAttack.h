#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#include "Wait.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class TaskAttack : public Wait {
private:
	sptr<Script_EnemyManager> mEnemyMgr;

public:
	TaskAttack(Object* object, float wait = 0.f, std::function<void()> callback = nullptr);
	virtual ~TaskAttack() = default;
};
#pragma endregion
