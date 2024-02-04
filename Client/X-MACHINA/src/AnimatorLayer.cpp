#include "stdafx.h"
#include "AnimatorLayer.h"

#include "AnimatorState.h"

AnimatorLayer::AnimatorLayer(std::string name, const std::vector<sptr<const AnimatorTransition>>& entryTransitions)
	:
	mName(name),
	mEntryTransitions(entryTransitions)
{

}

AnimatorLayer::AnimatorLayer(const AnimatorLayer& other)
{
	mName = other.mName;
	mEntryTransitions = other.mEntryTransitions;

	for (const auto& [name, state] : other.mStates) {
		AddState(std::make_shared<AnimatorState>(*state));
	}

	for (const auto& [name, layer] : other.mLayers) {
		AddLayer(std::make_shared<AnimatorLayer>(*layer));
	}
}

void AnimatorLayer::AddState(rsptr<AnimatorState> state)
{
	mStates.insert(std::make_pair(state->GetName(), state));
}


sptr<AnimatorState> AnimatorLayer::Entry() const
{
	if (mStates.contains(mEntryTransitions.front()->Destination)) {
		return mStates.at(mEntryTransitions.front()->Destination);
	}
	else if (mLayers.contains(mEntryTransitions.front()->Destination)) {
		return mLayers.at(mEntryTransitions.front()->Destination)->Entry();
	}

	throw std::runtime_error("There's no animator layer entry");
}

void AnimatorLayer::AddLayer(rsptr<AnimatorLayer> layer)
{
	mLayers.insert(std::make_pair(layer->GetName(), layer));
	layer->SetParent(this);
}