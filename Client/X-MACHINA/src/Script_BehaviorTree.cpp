#include "stdafx.h"
#include "Script_BehaviorTree.h"


void Script_BehaviorTree::Start()
{
	mRoot = SetupTree();
	mRoot->SetRoot();
}


void Script_BehaviorTree::Update()
{
	if (mRoot) {
		mRoot->Evaluate();
	}
}


void Script_BehaviorTree::OnDestroy()
{
	Release(mRoot);
}


void Script_BehaviorTree::Release(BT::Node* node)
{
	if (node == nullptr)
		return;

	for (auto& child : node->mChildren) {
		Release(child);
	}

	delete node;
	node = nullptr;
}
