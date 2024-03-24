#include "EnginePch.h"
#include "AnimatorMotion.h"

#include "AnimationClip.h"

AnimatorState::AnimatorState(rsptr<const AnimatorStateMachine> stateMachine, const std::vector<sptr<const AnimatorTransition>>& transitions, rsptr<const AnimationClip> clip)
	:
	AnimatorMotion(stateMachine, transitions, clip->mName, clip->mLength),
	AnimatorTrack(clip)
{

}

AnimatorState::AnimatorState(const AnimatorState& other)
	:
	AnimatorMotion(other),
	AnimatorTrack(other)
{

}

Vec4x4 AnimatorState::GetSRT(int boneIndex) const
{
	return AnimatorTrack::GetSRT(boneIndex, GetLength());
}
