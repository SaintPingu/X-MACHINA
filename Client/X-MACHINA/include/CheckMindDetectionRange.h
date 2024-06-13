#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class CheckMindDetectionRange : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;

public:
	CheckMindDetectionRange(Object* object);
	virtual ~CheckMindDetectionRange() = default;

	virtual BT::NodeState Evaluate() override;
};
#pragma endregion

