#pragma once

class Object;

namespace BehaviorTree {
	enum class NodeState {
		Running = 0,
		Success,
		Failure,
		Wait,
	};

	class Node {
	public:
		Node* mParent{};
		Node* mRoot{};
		std::queue<Node*> mWaitQueue{};
		std::vector<Node*> mChildren{};

	protected:
		Object* mObject{};

	private:
		std::unordered_map<std::string, sptr<Object>> mDataContext{};

	public:
		Node() {};
		Node(std::vector<Node*>& children);
		virtual ~Node() = default;

	public:
		virtual NodeState Evaluate() { return NodeState::Failure; };

		void SetRoot();
		void SetData(const std::string& key, sptr<Object> object);
		sptr<Object> GetData(const std::string& key);
		bool ClearData(std::string key);

	private:
		void Attach(Node* node);
	};


	class Sequence : public Node {
	public:
		Sequence() : Node() {}
		Sequence(std::vector<Node*>& children) : Node(children) {}

	public:
		virtual NodeState Evaluate() override;
	};


	class Selector : public Node {
	public:
		Selector() : Node() {}
		Selector(std::vector<Node*>& children) : Node(children) {}

	public:
		virtual NodeState Evaluate() override;
	};
}

namespace BT = BehaviorTree;