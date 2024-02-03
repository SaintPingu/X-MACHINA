#include "stdafx.h"
#include "AnimatorState.h"

#include "AnimationClip.h"
#include "Timer.h"


std::string AnimatorTransition::CheckTransition(const std::string& param, float value) const
{
	for (const auto& condition : Conditions) {
		if (condition.param != param) {
			continue;
		}

		switch (Hash(condition.mode)) {
		case Hash("If"):	// == true
			if (Math::IsEqual(value, 1.f)) {
				return Destination;
			}
			break;
		case Hash("IfNot"):	// == false
			if (Math::IsEqual(value, 0.f)) {
				return Destination;
			}
			break;
		default:
			assert(0);
			break;
		}
	}

	return "";
}

AnimatorState::AnimatorState(rsptr<const AnimationClip> clip, const std::vector<sptr<const AnimatorTransition>>& transitions)
	:
	mClip(clip),
	mName(clip->mName),
	mTransitions(transitions)
{

}

AnimatorState::AnimatorState(const AnimatorState& other)
{
	mName = other.mName;
	mClip = other.mClip;
	mSpeed = other.mSpeed;
	mCrntLength = other.mCrntLength;
	mWeight = other.mWeight;
	mTransitions = other.mTransitions;
}

Vec4x4 AnimatorState::GetSRT(int boneIndex) const
{
	return mClip->GetSRT(boneIndex, mCrntLength);
}

void AnimatorState::Init()
{
	mCrntLength = 0.f;
	mWeight = 1.f;
}

bool AnimatorState::Animate()
{
	mCrntLength += mSpeed * DeltaTime();
	if (IsEndAnimation()) {
		mCrntLength = 0.f;
		return true;
	}

	return false;
}


std::string AnimatorState::CheckTransition(const std::string& param, float value)
{
	for (const auto& transition : mTransitions) {
		std::string destination = transition->CheckTransition(param, value);
		if (destination != "") {
			return destination;
		}
	}

	return "";
}


bool AnimatorState::IsEndAnimation()
{
	return mCrntLength >= mClip->mLength;
}