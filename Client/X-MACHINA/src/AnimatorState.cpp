#include "stdafx.h"
#include "AnimatorState.h"

#include "AnimatorStateMachine.h"
#include "AnimationClip.h"
#include "Timer.h"

AnimatorState::AnimatorState(rsptr<const AnimatorStateMachine> staeMachine, rsptr<const AnimationClip> clip, const std::vector<sptr<const AnimatorTransition>>& transitions)
	:
	mStateMachine(staeMachine.get()),
	mClip(clip),
	mName(clip->mName),
	mTransitions(transitions)
{

}

AnimatorState::AnimatorState(const AnimatorState& other)
{
	mStateMachine = other.mStateMachine;
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

void AnimatorState::SetLength(float length)
{
	mCrntLength = length;
	if (IsEndAnimation()) {
		mCrntLength = 0.f;
	}
}

void AnimatorState::Init()
{
	mCrntLength = 0.f;
	mWeight = 1.f;
	mSpeed = 1.f;
}

bool AnimatorState::Animate()
{
	constexpr float corrSpeed = 0.5f;	// 스피드 보정값 (60fps)

	mCrntLength += (mSpeed * corrSpeed) * DeltaTime();
	if (IsEndAnimation()) {
		mCrntLength = 0.f;
		return true;
	}

	return false;
}

bool AnimatorState::IsEndAnimation()
{
	return (mCrntLength > mClip->mLength) || (mCrntLength < 0);
}