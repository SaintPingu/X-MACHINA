#include "EnginePch.h"
#include "AnimatorStateMachine.h"

#include "AnimatorController.h"
#include "AnimatorLayer.h"
#include "AnimatorMotion.h"

#pragma region AnimatorTransition
std::string AnimatorTransition::CheckTransition(const AnimatorController* controller) const
{
	if (Conditions.empty()) {
		return Destination;
	}

	for (const auto& condition : Conditions) {
		const AnimatorParameter* param = controller->GetParamRef(condition.paramName);
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
		case Hash("NotEqual"):	// == !Math::IsEqual
		{
			switch (param->type) {
			case AnimatorParameter::Type::Int:
			{
				int value = param->val.i;
				if (value == static_cast<int>(round(condition.threshold))) {
					return "";
				}
			}

			break;
			case AnimatorParameter::Type::Float:
			{
				float value = param->val.f;
				if (Math::IsEqual(value, condition.threshold)) {
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
		case Hash("Greater"):
		{
			switch (param->type) {
			case AnimatorParameter::Type::Int:
			{
				int value = param->val.i;
				if (value <= static_cast<int>(round(condition.threshold))) {
					return "";
				}
			}

			break;
			case AnimatorParameter::Type::Float:
			{
				float value = param->val.f;
				if (value <= condition.threshold) {
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
		case Hash("Less"):
		{
			switch (param->type) {
			case AnimatorParameter::Type::Int:
			{
				int value = param->val.i;
				if (value >= static_cast<int>(round(condition.threshold))) {
					return "";
				}
			}

			break;
			case AnimatorParameter::Type::Float:
			{
				float value = param->val.f;
				if (value >= condition.threshold) {
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


#pragma region AnimatorStateMachine
AnimatorStateMachine::AnimatorStateMachine(const std::string& name, const std::vector<AnimatorTransition>& entryTransitions)
	:
	mName(name),
	mEntryTransitions(entryTransitions)
{
}

AnimatorStateMachine::AnimatorStateMachine(const AnimatorStateMachine& other)
{
	mName = other.mName;
	mEntryTransitions = other.mEntryTransitions;

	for (const auto& [name, state] : other.mStates) {
		if (const auto& cast = std::dynamic_pointer_cast<AnimatorState>(state)) {
			AddState(std::make_shared<AnimatorState>(*cast));
		}
		else if (const auto& cast = std::dynamic_pointer_cast<BlendTree>(state)) {
			AddState(std::make_shared<BlendTree>(*cast));
		}
	}

	for (const auto& [name, stateMachine] : other.mStateMachines) {
		AddStateMachine(std::make_shared<AnimatorStateMachine>(*stateMachine));
	}
}

AnimatorMotion* AnimatorStateMachine::GetState(const std::string& name) const
{
	if (mStates.contains(name)) {
		return mStates.at(name).get();
	}

	return nullptr;
}
AnimatorStateMachine* AnimatorStateMachine::GetStateMachine(const std::string& name) const
{
	if (mStateMachines.contains(name)) {
		return mStateMachines.at(name).get();
	}

	return nullptr;
}

void AnimatorStateMachine::PushState(AnimatorMotion* motion) const
{
	mLayer->PushState(motion);
}

void AnimatorStateMachine::PushState(const std::string& motionName) const
{
	PushState(mStates.at(motionName).get());
	mLayer->GetController()->CheckTransition();
}

void AnimatorStateMachine::Init(const AnimatorController* controller, AnimatorLayer* layer)
{
	mLayer = layer;
	for (auto& [name, state] : mStates) {
		state->Init(controller);
	}

	for (auto& [name, subState] : mStateMachines) {
		subState->Init(controller, layer);
	}
}

void AnimatorStateMachine::Release()
{
	for (const auto& state : mStates) {
		const auto& motion = state.second;
		motion->Release();
	}

	for (const auto& [name, stateMachine] : mStateMachines) {
		stateMachine->Release();
	}

	mStates.clear();
	mStateMachines.clear();
	mEntryTransitions.clear();
}

void AnimatorStateMachine::AddState(rsptr<AnimatorMotion> state)
{
	mStates.insert(std::make_pair(state->GetName(), state));
	state->Init(this);
}

AnimatorMotion* AnimatorStateMachine::Entry() const
{
	if (mStates.contains(mEntryTransitions.front().Destination)) {
		return mStates.at(mEntryTransitions.front().Destination).get();
	}
	else if (mStateMachines.contains(mEntryTransitions.front().Destination)) {
		return mStateMachines.at(mEntryTransitions.front().Destination)->Entry();
	}

	throw std::runtime_error("There's no animator state machine's entry");
}

void AnimatorStateMachine::AddStateMachine(rsptr<AnimatorStateMachine> stateMachine)
{
	mStateMachines.insert(std::make_pair(stateMachine->GetName(), stateMachine));
	stateMachine->SetParent(this);
}

AnimatorMotion* AnimatorStateMachine::CheckTransition(const AnimatorController* controller) const
{
	for (const auto& transition : mEntryTransitions) {
		std::string destination = transition.CheckTransition(controller);
		if (destination != "") {
			if (AnimatorMotion* state = GetState(destination)) {
				return state;
			}

			if (AnimatorStateMachine* subStateMachine = GetStateMachine(destination)) {
				return subStateMachine->CheckTransition(controller);
			}
		}
	}

	return nullptr;
}

AnimatorMotion* AnimatorStateMachine::FindMotionByName(const std::string& motionName) const
{
	for (const auto& state : mStates) {
		const auto& motion = state.second;
		if (motion->GetName() == motionName) {
			return motion.get();
		}
	}

	for (const auto& stateMachine : mStateMachines) {
		if (auto motion = stateMachine.second->FindMotionByName(motionName)) {
			return motion;
		}
	}

	return nullptr;
}
void AnimatorStateMachine::AddStates(int& index, std::unordered_map<int, std::string>& motionMapInt, std::unordered_map<std::string, int>& motionMapString)
{
	for (auto& state : mStates) {
		motionMapInt.insert(std::make_pair(index, state.first));
		motionMapString.insert(std::make_pair(state.first, index++));
	}

	for (auto& stateMachine : mStateMachines) {
		stateMachine.second->AddStates(index, motionMapInt, motionMapString);
	}
}
#pragma endregion