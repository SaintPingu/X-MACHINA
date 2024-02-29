#include "stdafx.h"
#include "AnimatorLayer.h"

#include "AnimatorController.h"
#include "AnimatorState.h"
#include "AnimatorStateMachine.h"


#pragma region AnimatorLayer
AnimatorLayer::AnimatorLayer(const std::string& name, sptr<AnimatorStateMachine> rootStateMachine, HumanBone boneMask)
	:
	mName(name),
	mRootStateMachine(rootStateMachine),
	mBoneMask(boneMask)
{
	mCrntState = mRootStateMachine->Entry();
}

AnimatorLayer::AnimatorLayer(const AnimatorLayer& other)
{
	mName = other.mName;
	mBoneMask = other.mBoneMask;
	mRootStateMachine = std::make_shared<AnimatorStateMachine>(*other.mRootStateMachine);
	mCrntState = mRootStateMachine->Entry();
}

Vec4x4 AnimatorLayer::GetTransform(int boneIndex, HumanBone boneType) const
{
	Vec4x4 transform = Matrix4x4::Scale(mCrntState->GetSRT(boneIndex), mCrntState->GetWeight());
	if (mNextState) {
		transform = Matrix4x4::Add(transform, Matrix4x4::Scale(mNextState->GetSRT(boneIndex), mNextState->GetWeight()));
	}

	return transform;
}

void AnimatorLayer::Animate()
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


void AnimatorLayer::CheckTransition(const AnimatorController* controller)
{
	if (mNextState) {
		return;
	}

	mNextState = mRootStateMachine->CheckTransition(controller);
	if (mNextState == mCrntState) {
		mNextState = nullptr;
	}
	else if (mNextState != nullptr) {
		mNextState->Init();
	}
}

void AnimatorLayer::ChangeToNextState()
{
	if (!mNextState) {
		return;
	}

	mCrntState->Init();
	mNextState->SetWeight(1.f);
	mCrntState = mNextState;
	mNextState = nullptr;
}
#pragma endregion