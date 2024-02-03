#pragma once

struct AnimatorTransition;
class AnimatorState;
class AnimatorLayer;
class AnimatorStateMachine;

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
	AnimatorLayer(std::string name, const Animations::StateMap& states, const Animations::LayerMap& layers);
	virtual ~AnimatorLayer() = default;

	sptr<AnimatorState> GetState(const std::string& name) const { return mStates.at(name); }

	void SetEntry(const std::vector<sptr<const AnimatorTransition>>& entryTransitions) { mEntryTransitions = entryTransitions; }

public:
	sptr<AnimatorState> Entry() const;
};