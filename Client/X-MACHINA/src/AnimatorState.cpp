#include "stdafx.h"
#include "AnimatorState.h"

#include "AnimationClip.h"
#include "Timer.h"

AnimatorState::AnimatorState(rsptr<const AnimationClip> clip, const std::vector<AnimatorTransition>& transitions)
	:
	mClip(clip),
	mName(clip->mName),
	mTransitions(transitions)
{

}

Vec4x4 AnimatorState::GetSRT(int boneIndex) const
{
	return mClip->GetSRT(boneIndex, mCrntLength);
}

void AnimatorState::Init()
{
	mCrntLength = 0.f;
}

int AnimatorState::Animate()
{
	constexpr float kTransitionDuration = 0.25f;

	mCrntLength += mSpeed * DeltaTime();
	if (IsEndAnimation()) {
		mCrntLength = 0.f;
	}

	return -1;
}


std::string AnimatorState::CheeckTransition(const std::string& param, float value)
{
	for (const auto& transition : mTransitions) {
		for (const auto& condition : transition.Conditions) {
			if (condition.param != param) {
				continue;
			}

			switch (Hash(condition.mode)) {
			case Hash("If"):	// == true
				if (Math::IsEqual(value, 1.f)) {
					return transition.Destination;
				}
				break;
			case Hash("IfNot"):	// == false
				if (Math::IsEqual(value, 0.f)) {
					return transition.Destination;
				}
				break;
			default:
				assert(0);
				break;
			}
		}
	}

	return "";
}


bool AnimatorState::IsEndAnimation()
{
	return mCrntLength > mClip->mLength;
}