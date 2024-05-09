#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class CheckDetectionRange : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;
	sptr<Object> mTarget;

public:
	CheckDetectionRange(Object* object);
	virtual ~CheckDetectionRange() = default;

	virtual BT::NodeState Evaluate() override;
};
#pragma endregion

