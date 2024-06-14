#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class Object;
#pragma endregion


#pragma region Class
class Script_MindControlledEnemyBT : public Script_BehaviorTree {
	COMPONENT(Script_MindControlledEnemyBT, Script_BehaviorTree)

private:
	Object* mInvoker{};
	float mShieldBuffAmount{};

public:
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;

public:
	void SetInvoker(Object* invoker) { mInvoker = invoker; }

protected:
	BT::Node* SetupTree() override;
};
#pragma endregion

