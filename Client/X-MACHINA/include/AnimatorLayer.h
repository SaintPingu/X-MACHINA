#pragma once

class AnimatorState;
class AnimatorLayer;
class AnimatorController;

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
	using StateMap = std::unordered_map<std::string, sptr<AnimatorState>>;
	using LayerMap = std::unordered_map<std::string, sptr<AnimatorLayer>>;
}

class AnimatorLayer {
private:
	std::string mName{};
	AnimatorLayer* mParent{};

	std::vector<sptr<const AnimatorTransition>> mEntryTransitions{};

	Animations::StateMap mStates{};
	Animations::LayerMap mLayers{};

public:
	AnimatorLayer(std::string name, const std::vector<sptr<const AnimatorTransition>>& entryTransitions);
	AnimatorLayer(const AnimatorLayer& other);
	virtual ~AnimatorLayer() = default;

	std::string GetName() const { return mName; }
	sptr<AnimatorState> GetState(const std::string& name) const;
	sptr<AnimatorLayer> GetLayer(const std::string& name) const;

	void SetParent(AnimatorLayer* parent) { mParent = parent; }

public:
	sptr<AnimatorState> Entry() const;

	void AddState(rsptr<AnimatorState> state);
	void AddLayer(rsptr<AnimatorLayer> layer);

	sptr<AnimatorState> CheckTransition(const AnimatorController* controller) const;
};