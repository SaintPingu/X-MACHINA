#include "stdafx.h"
#include "AnimatorController.h"

#include "Animator.h"
#include "AnimatorState.h"
#include "AnimatorLayer.h"
#include "AnimationClip.h"
#include "Scene.h"
#include "Timer.h"

AnimatorController::AnimatorController(const Animations::ParamMap& parameters, rsptr<AnimatorLayer> baseLayer)
	:
	mParameters(parameters),
	mBaseLayer(baseLayer)
{
	mCrntLayer = mBaseLayer;
	mCrntState = mCrntLayer->Entry();
}

AnimatorController::AnimatorController(const AnimatorController& other)
	:
	mParameters(other.mParameters),
	mBaseLayer(std::make_shared<AnimatorLayer>(*other.mBaseLayer))
{
	mCrntLayer = mBaseLayer;
	mCrntState = mCrntLayer->Entry();
}

void AnimatorController::Animate()
{
	mCrntState->Animate();

	if (mNextState) {
		bool isEndAnimation = mNextState->Animate();

		if (isEndAnimation) {
			ChangeToNextState();
		}
		else {
			constexpr float kMaxDuration = .25f;
			const float crntDuration = mNextState->GetCrntLength();

			const float t = crntDuration / kMaxDuration;
			if (t < 1.f) {
				mCrntState->SetWeight(1 - t);
				mNextState->SetWeight(t);
			}
			else {
				ChangeToNextState();
			}
		}
	}
}

Vec4x4 AnimatorController::GetTransform(int boneIndex)
{
	Vec4x4 transform = Matrix4x4::Scale(mCrntState->GetSRT(boneIndex), mCrntState->GetWeight());
	if (mNextState) {
		transform = Matrix4x4::Add(transform, Matrix4x4::Scale(mNextState->GetSRT(boneIndex), mNextState->GetWeight()));
	}

	return transform;
}

void AnimatorController::SetBool(const std::string& name, bool value)
{
	if (mNextState) {
		return;
	}
	if (!mParameters.contains(name)) {
		return;
	}

	mParameters[name].val.b = value;

	mNextState = mCrntState->CheckTransition(name, value);
	if (mNextState != nullptr) {
		mNextState->Init();
	}
}


void AnimatorController::ChangeToNextState()
{
	if (!mNextState) {
		return;
	}

	mCrntState->Init();
	mNextState->SetWeight(1.f);
	mCrntState = mNextState;
	mNextState = nullptr;
}