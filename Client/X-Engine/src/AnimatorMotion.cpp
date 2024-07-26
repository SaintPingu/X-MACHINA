#include "EnginePch.h"
#include "AnimatorMotion.h"

#include "AnimatorStateMachine.h"
#include "AnimationClip.h"
#include "AnimatorLayer.h"
#include "AnimatorController.h"
#include "Timer.h"

AnimatorMotion::AnimatorMotion(const AnimatorMotionInfo& info)
	:
	mName(info.Name),
	mOriginSpeed(info.Speed),
	mCrntSpeed(info.Speed),
	mCrntLength(0),
	mMaxLength(info.Length),
	mWeight(1)
{
}

AnimatorMotion::AnimatorMotion(const AnimatorMotion& other)
	:
	mName(other.mName),
	mOriginSpeed(other.mOriginSpeed),
	mCrntSpeed(other.mCrntSpeed),
	mCrntLength(other.mCrntLength),
	mMaxLength(other.mMaxLength),
	mWeight(other.mWeight)
{
}

void AnimatorMotion::Release()
{
	mCallbacks.clear();
	mCallbackStop = nullptr;
	mCallbackChange = nullptr;
}

void AnimatorMotion::ResetLength()
{
	if (IsReverse()) {
		mCrntLength = mMaxLength;
	}
	else {
		mCrntLength = 0.f;
	}

	ResetCallbacks();
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
	mCrntSpeed  = mOriginSpeed;
	mIsReverse  = 1;

	ResetCallbacks();
}


bool AnimatorMotion::IsEndAnimation() const
{
	return (mCrntLength >= mMaxLength) || (mCrntLength <= 0);
}

bool AnimatorMotion::IsSameStateMachine(const AnimatorMotion* other) const
{
	return mStateMachine == other->mStateMachine;
}

bool AnimatorMotion::Animate()
{
	mCrntLength += (mCrntSpeed * mIsReverse) * DeltaTime();

	if (mCallbackAnimate) {
		mCallbackAnimate->Callback();
	}

	for (auto& [time, callback] : mCallbacks | std::ranges::views::reverse) {
		if (mCrntLength >= time) {
			if (!callback.Called) {
				if (callback.Callback) {
					callback.Callback();
					callback.Called = true;
				}
			}
			break;
		}
	}


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



void AnimatorMotion::AddCallback(const std::function<void()>& callback, int frame)
{
	mCallbacks.insert(std::make_pair(GetFrameTime(frame), MotionCallback{ callback }));
}

void AnimatorMotion::DelCallback(int frame)
{
	mCallbacks.erase(GetFrameTime(frame));
}

void AnimatorMotion::AddStartCallback(const std::function<void()>& callback)
{
	AddCallback(callback, 0);
}

void AnimatorMotion::AddEndCallback(const std::function<void()>& callback)
{
	AddCallback(callback, GetMaxFrameRate());
}

void AnimatorMotion::AddStopCallback(const std::function<void()>& callback)
{
	mCallbackStop = std::make_shared<MotionCallback>(callback);
}

void AnimatorMotion::DelStopCallback()
{
	mCallbackStop = nullptr;
}

void AnimatorMotion::AddChangeCallback(const std::function<void()>& callback)
{
	mCallbackChange = std::make_shared<MotionCallback>(callback);
}

void AnimatorMotion::DelChabgeCallback()
{
	mCallbackChange = nullptr;
}

void AnimatorMotion::AddAnimateCallback(const std::function<void()>& callback)
{
	mCallbackAnimate = std::make_shared<MotionCallback>(callback);
}

void AnimatorMotion::DelAnimateCallback()
{
	mCallbackAnimate = nullptr;
}

void AnimatorMotion::StopAnimation()
{
	if (mCallbackStop) {
		mCallbackStop->Callback();
	}
	Reset();
}

void AnimatorMotion::CallbackChange()
{
	if (mCallbackChange) {
		mCallbackChange->Callback();
	}
}



void AnimatorMotion::ResetCallbacks()
{
	for (auto& callback : mCallbacks) {
		callback.second.Reset();
	}
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

Matrix AnimatorTrack::GetSRT(const std::string& boneName, float length) const
{
	return mClip->GetSRT(boneName, length);
}