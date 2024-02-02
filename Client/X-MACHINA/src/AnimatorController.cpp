#include "stdafx.h"
#include "AnimatorController.h"

#include "Animator.h"
#include "AnimatorState.h"
#include "Scene.h"

AnimatorController::AnimatorController(const std::vector<AnimatorParameter>& parameters, const std::vector<sptr<AnimatorState>>& states)
	:
	mParameters(parameters),
	mStates(states)
{
	mCrntState = mStates.front();
}

AnimatorController::AnimatorController(const AnimatorController& other)
	:
	mParameters(other.mParameters),
	mStates(other.mStates),
	mCrntState(other.mCrntState)
{

}

//void AnimatorController::Init(rsptr<const AnimationLoadInfo> animationInfo)
//{
//	const auto& animationClips = scene->GetAnimationClips(animationInfo->AnimationClipFolder);
//
//	mStates.resize(animationClips.size());
//	for (int i = 0; i < animationClips.size(); ++i) {
//		mStates[i] = std::make_shared<AnimatorState>();
//		mStates[i]->SetAnimationClip(animationClips[i]);
//	}
//
//	mCrntState = mStates.front();
//}

void AnimatorController::Animate()
{
	mCrntState->Animate();
}

Vec4x4 AnimatorController::GetTransform(int boneIndex)
{
	return Matrix4x4::Scale(mCrntState->GetSRT(boneIndex), mCrntState->GetWeight());
}