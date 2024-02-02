#include "stdafx.h"
#include "AnimatorState.h"

#include "AnimationClip.h"
#include "Timer.h"

AnimatorState::AnimatorState(const std::string name, rsptr<const AnimationClip> clip, const std::vector<AnimatorTransition>& transitions)
	:
	mName(name),
	mClip(clip),
	mTransitions(transitions)
{

}

Vec4x4 AnimatorState::GetSRT(int boneIndex) const
{
	return mClip->GetSRT(boneIndex, mCrntLength);
}

void AnimatorState::Init()
{
	mNextTrack = nullptr;
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



bool AnimatorState::IsEndAnimation()
{
	return mCrntLength > mClip->mLength;
}