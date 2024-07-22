#pragma once

class AnimatorMotion;
class AnimatorStateMachine;
class AnimatorController;
class AnimatorLayer;

// AnimatorState간 상태 전이 조건
struct AnimationCondition {
	std::string mode{};
	std::string paramName{};
	float threshold{};
};

// AnimatorState간 상태 전이를 관리한다.
struct AnimatorTransition {
	std::string Destination{};
	std::vector<AnimationCondition> Conditions{};

	std::string CheckTransition(const AnimatorController* controller) const;
};

namespace Animations {
	using StateMap = std::unordered_map<std::string, sptr<AnimatorMotion>>;
	using StateMachineMap = std::unordered_map<std::string, sptr<AnimatorStateMachine>>;
}

class AnimatorStateMachine {
private:
	std::string mName{};
	AnimatorLayer* mLayer{};
	AnimatorStateMachine* mParent{};

	std::vector<AnimatorTransition> mEntryTransitions{};

	Animations::StateMap mStates{};
	Animations::StateMachineMap mStateMachines{};

public:
	AnimatorStateMachine(const std::string& name, const std::vector<AnimatorTransition>& entryTransitions);
	AnimatorStateMachine(const AnimatorStateMachine& other);
	virtual ~AnimatorStateMachine() = default;

public:
	std::string GetName() const { return mName; }
	AnimatorMotion* GetState(const std::string& name) const;
	AnimatorStateMachine* GetStateMachine(const std::string& name) const;
	const AnimatorLayer* GetLayer() const { return mLayer; }

	void SetParent(AnimatorStateMachine* parent) { mParent = parent; }
	void PushState(AnimatorMotion* motion) const;
	void PushState(const std::string& motionName) const;

public:
	void Init(const AnimatorController* controller, AnimatorLayer* layer);
	void Release();

	void AddState(rsptr<AnimatorMotion> state);
	void AddStateMachine(rsptr<AnimatorStateMachine> stateMachine);

	AnimatorMotion* Entry() const;
	AnimatorMotion* CheckTransition(const AnimatorController* controller) const;
	AnimatorMotion* FindMotionByName(const std::string& motionName) const;

	void AddStates(int& index, std::unordered_map<int, std::string>& motionMapInt, std::unordered_map<std::string, int>& motionMapString);
};