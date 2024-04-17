#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class Wait : public BT::Node {
private:
	float mAccTime{};
	float mWaitTime{};

public:
	Wait(float waitTime) : mWaitTime(waitTime) {}
	virtual ~Wait() = default;

public:
	virtual BT::NodeState Evaluate() override;
};
#pragma endregion
