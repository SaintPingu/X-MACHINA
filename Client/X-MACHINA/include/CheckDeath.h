#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
class Script_LiveObject;
#pragma endregion


#pragma region Class
class CheckDeath : public BT::ActionNode {
private:
	sptr<Script_EnemyManager> mEnemyMgr;
	sptr<Script_LiveObject> mLiveObject;
	float mAccTime{}; 
	float mRemoveTime{};

public:
	CheckDeath(Object* object, const std::function<void()>& callback = nullptr);
	virtual ~CheckDeath() = default;

public:
	virtual BT::NodeState Evaluate() override;

private:
	void DeathEndCallback();
};
#pragma endregion
