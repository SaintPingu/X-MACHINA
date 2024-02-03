#include "stdafx.h"
#include "AnimatorLayer.h"

#include "AnimatorState.h"

AnimatorLayer::AnimatorLayer(std::string name, const Animations::StateMap& states, const Animations::LayerMap& layers)
	:
	mName(name),
	mStates(states),
	mLayers(layers)
{

}


sptr<AnimatorState> AnimatorLayer::Entry() const
{
	return mStates.at(mEntryTransitions.front()->Destination);
}