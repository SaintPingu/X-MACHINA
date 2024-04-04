#pragma once

#pragma region Include
#include "Component/Component.h"
#include "BehaviorTree.h"
#pragma endregion


#pragma region Class
class Script_BehaviorTree : public Component {
	COMPONENT_ABSTRACT(Script_BehaviorTree, Component)

private:
	BT::Node* mRoot{};

public:
	virtual void Start() override;
	virtual void Update() override;
	virtual void OnDestroy() override;

protected:
	virtual BT::Node* SetupTree() abstract;
	
private:
	void Release(BT::Node* node);
};
#pragma endregion