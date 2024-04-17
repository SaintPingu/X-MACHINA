#include "EnginePch.h"
#include "BehaviorTree.h"


namespace BehaviorTree {

	Node::Node(std::vector<Node*>& children)
	{
		for (auto& child : children) {
			Attach(child);
		}
	}


	void Node::SetRoot()
	{
		mRoot = this;

		std::queue<Node*> q;
		q.push(mRoot);

		while (!q.empty()) {
			Node* currNode = q.front();
			q.pop();

			for (auto& child : currNode->mChildren) {
				child->mRoot = this;
				q.push(child);
			}
		}
	}


	void Node::SetData(const std::string& key, sptr<Object> object)
	{
		mDataContext[key] = object;
	}


	sptr<Object> Node::GetData(const std::string& key)
	{
		auto mValue = mDataContext.find(key);
		if (mValue != mDataContext.end())
			return mValue->second;

		Node* parent = mParent;
		while (true) {
			if (!parent)
				break;

			if (auto pValue = parent->GetData(key))
				return pValue;

			parent = parent->mParent;
		}

		return nullptr;
	}


	bool Node::ClearData(std::string key)
	{
		if (mDataContext.contains(key)) {
			mDataContext.erase(key);
			return true;
		}

		Node* parent = mParent;
		while (!parent) {
			if (!parent)
				break;

			bool clear = parent->ClearData(key);
			if (clear)
				return true;

			parent = parent->mParent;
		}

		return false;
	}


	void Node::Attach(Node* node)
	{
		node->mParent = this;
		mChildren.emplace_back(node);
	}


	NodeState Sequence::Evaluate()
	{
		bool isRunning = false;

		if (!mRoot->mWaitQueue.empty()) {
			if (mRoot->mWaitQueue.front()->Evaluate() != NodeState::Wait)
				mRoot->mWaitQueue.pop();

			return NodeState::Success;
		}
		
		for (const auto& child : mChildren) {
			switch (child->Evaluate())
			{
			case NodeState::Failure:
				return NodeState::Failure;
			case NodeState::Success:
				continue;
			case NodeState::Running:
				isRunning = true;
				continue;
			case NodeState::Wait:
				mRoot->mWaitQueue.push(child);
				return NodeState::Success;
			default:
				return NodeState::Success;
			}
		}

		return isRunning ? NodeState::Running : NodeState::Success;
	}


	NodeState Selector::Evaluate()
	{
		if (!mRoot->mWaitQueue.empty()) {
			if (mRoot->mWaitQueue.front()->Evaluate() != NodeState::Wait)
				mRoot->mWaitQueue.pop();

			return NodeState::Failure;
		}

		for (const auto& child : mChildren) {
			switch (child->Evaluate())
			{
			case NodeState::Failure:
				continue;
			case NodeState::Success:
				return NodeState::Success;
			case NodeState::Running:
				return NodeState::Running;
			case NodeState::Wait:
				mRoot->mWaitQueue.push(child);
				return NodeState::Success;
			default:
				continue;
			}
		}

		return NodeState::Failure;
	}
}