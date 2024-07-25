#include "EnginePch.h"
#include "AnimatorMotion.h"

#include "AnimationClip.h"

AnimatorState::AnimatorState(const AnimatorMotionInfo& info, rsptr<const AnimationClip> clip)
	:
	AnimatorMotion(AnimatorMotionInfo{ clip->mName, clip->mLength, info.Speed }),
	AnimatorTrack(clip)
{

}

AnimatorState::AnimatorState(const AnimatorState& other)
	:
	AnimatorMotion(other),
	AnimatorTrack(other)
{

}

Matrix AnimatorState::GetSRT(const std::string& boneName) const
{
	return AnimatorTrack::GetSRT(boneName, GetLength());
}

int AnimatorState::GetMaxFrameRate() const
{
	return AnimatorTrack::GetClip()->GetMaxFrameRate();
}

sptr<const AnimationClip> AnimatorState::GetClip() const
{
	return AnimatorTrack::GetClip();
}

float AnimatorState::GetFrameTime(int frame)
{
	return GetClip()->GetFrameTime(frame);
}
