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
	std::function<void()> mWaitingCallback;

public:
	Wait(float waitTime, std::function<void()> callback = nullptr);
	virtual ~Wait() = default;

public:
	virtual BT::NodeState Evaluate() override;

protected:
	void SetWait(float wait) { mWaitTime = wait; }
};
#pragma endregion
