#pragma once

struct AnimatorTransition;
class AnimatorState;
class AnimatorLayer;

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
	sptr<AnimatorState> GetState(const std::string& name) const { return mStates.at(name); }

	void AddState(rsptr<AnimatorState> state);
	void SetParent(AnimatorLayer* parent) { mParent = parent; }

public:
	sptr<AnimatorState> Entry() const;

	void AddLayer(rsptr<AnimatorLayer> layer);
};