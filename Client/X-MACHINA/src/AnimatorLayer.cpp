#include "stdafx.h"
#include "AnimatorLayer.h"

#include "Timer.h"
#include "AnimationClip.h"
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

void AnimatorLayer::SetCrntStateLength(float length) const
{
	if (mNextState) {
		return;
	}

	mCrntState->SetLength(length);
}

void AnimatorLayer::Animate()
{
	mCrntState->Animate();
	if (mSyncState) {
		float distance = fabs(mCrntState->GetCrntLength() - mSyncState->GetCrntLength());
		if (fabs(distance) <= 0.05f) {
			SyncComplete();
		}
	}

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
		mCrntState->SetSpeed(1);
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

	if (mController) {
		mController->SyncAnimation();
	}
}

void AnimatorLayer::SyncAnimation(rsptr<const AnimatorState> srcState)
{
	if (mNextState) {
		return;
	}

	mSyncState = srcState;

	float length = mSyncState->GetCrntLength();
	float threshold = mSyncState->GetClip()->mLength;
	float myLength = mCrntState->GetCrntLength();
	float distance = length - myLength;
	if (fabs(distance) < 0.05f) {
		SyncComplete();
		return;
	}


	float myThreshold = mCrntState->GetClip()->mLength;
	threshold = threshold < myThreshold ? threshold : myThreshold;

	float speed{};

	float otherDstTime = threshold - length;	// 대상의 애니메이션 종료 시간
	float myDstTime = threshold - myLength;		// 나의         "
	// 남은 길이가 절반 이상이라면
	if (fabs(distance) > threshold / 2) {
		//speed = 0;
		speed = otherDstTime / myDstTime;
	}
	else {
		// 내 애니메이션이 빠를 경우 천천히,
		//				  느릴 경우 빨리
		// 재생해 두 애니메이션 속도를 맞춘다.

		// 나의 애니메이션 종료 시간이 대상과 동일해야 한다.
		speed = myDstTime / otherDstTime;
	}

	speed = std::clamp(speed, 0.75f, 1.25f);

	mCrntState->SetSpeed(speed);
	//mCrntState->SetLength(length);
}

void AnimatorLayer::SyncComplete()
{
	mCrntState->SetSpeed(1);
	float distance = mCrntState->GetCrntLength() - mSyncState->GetCrntLength();
	if (distance > 0) {
		int a = 5;
	}
	mCrntState->SetLength(mSyncState->GetCrntLength());
	mSyncState = nullptr;
}
#pragma endregion