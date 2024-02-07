#include "stdafx.h"
#include "AnimatorLayer.h"

#include "AnimatorController.h"
#include "AnimatorState.h"

#pragma region AnimatorTransition
std::string AnimatorTransition::CheckTransition(const AnimatorController* controller) const
{
	if (Conditions.empty()) {
		return Destination;
	}

	for (const auto& condition : Conditions) {
		const AnimatorParameter* param = controller->GetParam(condition.paramName);
		if (!param) {
			continue;
		}

		switch (Hash(condition.mode)) {
		case Hash("If"):	// == true
		{
			bool value = param->val.b;
			if (value != true) {
				return "";
			}
		}

		break;
		case Hash("IfNot"):	// == false
		{
			bool value = param->val.b;
			if (value != false) {
				return "";
			}
		}

		break;
		case Hash("Equals"):	// == Math::IsEqual
		{
			switch (param->type) {
			case AnimatorParameter::Type::Int:
			{
				int value = param->val.i;
				if (value != static_cast<int>(round(condition.threshold))) {
					return "";
				}
			}

			break;
			case AnimatorParameter::Type::Float:
			{
				float value = param->val.f;
				if (!Math::IsEqual(value, condition.threshold)) {
					return "";
				}
			}

			break;
			default:
				assert(0);
				break;
			}
		}

		break;
		default:
			assert(0);
			break;
		}
	}

	return Destination;
}
#pragma endregion



#pragma region AnimatorLayer
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

sptr<AnimatorState> AnimatorLayer::GetState(const std::string& name) const
{
	if (mStates.contains(name)) {
		return mStates.at(name);
	}
	
	return nullptr;
}
sptr<AnimatorLayer> AnimatorLayer::GetLayer(const std::string& name) const
{
	if (mLayers.contains(name)) {
		return mLayers.at(name);
	}

	return nullptr;
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

sptr<AnimatorState> AnimatorLayer::CheckTransition(const AnimatorController* controller) const
{
	for (const auto& transition : mEntryTransitions) {
		std::string destination = transition->CheckTransition(controller);
		if (destination != "") {
			sptr<AnimatorState> state = GetState(destination);
			if (state) {
				return state;
			}

			sptr<AnimatorLayer> subLayer = GetLayer(destination);
			if (subLayer) {
				return subLayer->CheckTransition(controller);
			}
		}
	}

	return nullptr;
}
#pragma endregion
