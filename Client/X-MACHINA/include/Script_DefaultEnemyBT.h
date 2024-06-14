#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region Class
class Script_DefaultEnemyBT : public Script_BehaviorTree {
	COMPONENT(Script_DefaultEnemyBT, Script_BehaviorTree)

public:
	virtual void OnEnable() override;

protected:
	BT::Node* SetupTree() override;
};
#pragma endregion

