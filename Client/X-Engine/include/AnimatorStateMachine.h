#pragma once

class AnimatorMotion;
class AnimatorStateMachine;
class AnimatorController;

// AnimatorState�� ���� ���� ����
struct AnimationCondition {
	std::string mode{};
	std::string paramName{};
	float threshold{};
};

// AnimatorState�� ���� ���̸� �����Ѵ�.
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
	AnimatorStateMachine* mParent{};

	std::vector<sptr<const AnimatorTransition>> mEntryTransitions{};

	Animations::StateMap mStates{};
	Animations::StateMachineMap mStateMachines{};

public:
	AnimatorStateMachine(const std::string& name, const std::vector<sptr<const AnimatorTransition>>& entryTransitions);
	AnimatorStateMachine(const AnimatorStateMachine& other);
	virtual ~AnimatorStateMachine() = default;

public:
	std::string GetName() const { return mName; }
	sptr<AnimatorMotion> GetState(const std::string& name) const;
	sptr<AnimatorStateMachine> GetStateMachine(const std::string& name) const;

	void SetParent(AnimatorStateMachine* parent) { mParent = parent; }

public:
	void Init(const AnimatorController* controller);

	void AddState(rsptr<AnimatorMotion> state);
	void AddStateMachine(rsptr<AnimatorStateMachine> stateMachine);

	sptr<AnimatorMotion> Entry() const;
	sptr<AnimatorMotion> CheckTransition(const AnimatorController* controller) const;
	sptr<AnimatorMotion> FindMotionByName(const std::string& motionName) const;
};