#include "stdafx.h"
#include "AnimatorMotion.h"

#include "AnimatorStateMachine.h"
#include "AnimationClip.h"
#include "Timer.h"

AnimatorMotion::AnimatorMotion(rsptr<const AnimatorStateMachine> stateMachine, const std::vector<sptr<const AnimatorTransition>>& transitions, const std::string& name, float maxLength)
	:
	mName(name),
	mMaxLength(maxLength),
	mStateMachine(stateMachine.get()),
	mTransitions(transitions)
{

}

AnimatorMotion::AnimatorMotion(const AnimatorMotion& other)
{
	mName         = other.mName;
	mSpeed        = other.mSpeed;
	mCrntLength   = other.mCrntLength;
	mMaxLength    = other.mMaxLength;
	mWeight       = other.mWeight;
	mStateMachine = other.mStateMachine;
	mTransitions  = other.mTransitions;
}

void AnimatorMotion::ResetLength()
{
	if (IsReverse()) {
		mCrntLength = mMaxLength;
	}
	else {
		mCrntLength = 0.f;
	}
}

void AnimatorMotion::SetLength(float length)
{
	mCrntLength = length;
	if (IsEndAnimation()) {
		ResetLength();
	}
}

void AnimatorMotion::Reset()
{
	mCrntLength = 0.f;
	mWeight     = 1.f;
	mSpeed      = 1.f;
	mIsReverse  = 1;
}


bool AnimatorMotion::IsEndAnimation() const
{
	return (mCrntLength >= mMaxLength) || (mCrntLength <= 0);
}

bool AnimatorMotion::IsSameStateMachine(rsptr<const AnimatorMotion> other) const
{
	return mStateMachine == other->mStateMachine;
}

bool AnimatorMotion::Animate()
{
	mCrntLength += (mSpeed * mIsReverse) * DeltaTime();
	if (IsEndAnimation()) {
		ResetLength();
		return true;
	}

	return false;
}

void AnimatorMotion::DecWeight()
{
	mWeight -= DeltaTime() * mkTransitionSpeed;
}

void AnimatorMotion::IncWeight()
{
	mWeight += DeltaTime() * mkTransitionSpeed;
}

AnimatorTrack::AnimatorTrack(rsptr<const AnimationClip> clip)
	:
	mClip(clip)
{

}

AnimatorTrack::AnimatorTrack(const AnimatorTrack& other)
{
	mClip = other.mClip;
}

Vec4x4 AnimatorTrack::GetSRT(int boneIndex, float length) const
{
	return mClip->GetSRT(boneIndex, length);
}