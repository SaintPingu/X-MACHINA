#include "EnginePch.h"
#include "AnimatorMotion.h"

#include "AnimationClip.h"

AnimatorState::AnimatorState(const AnimatorMotionInfo& info, rsptr<const AnimationClip> clip)
	:
	AnimatorMotion(AnimatorMotionInfo{ clip->mName, clip->mLength, info.Speed, info.StateMachine, info.Transitions }),
	AnimatorTrack(clip)
{

}

AnimatorState::AnimatorState(const AnimatorState& other)
	:
	AnimatorMotion(other),
	AnimatorTrack(other)
{

}

Matrix AnimatorState::GetSRT(int boneIndex) const
{
	return AnimatorTrack::GetSRT(boneIndex, GetLength());
}

float AnimatorState::GetFrameTime(int frame)
{
	return GetClip()->GetFrameTime(frame);
}
