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
	for (auto& nextState : mNextStates) {
		transform = Matrix4x4::Add(transform, Matrix4x4::Scale(nextState->GetSRT(boneIndex), nextState->GetWeight()));
	}

	return transform;
}

void AnimatorLayer::SetCrntStateLength(float length) const
{
	if (!mNextStates.empty()) {
		return;
	}

	mCrntState->SetLength(length);
}


void AnimatorLayer::Init(const AnimatorController* controller)
{
	mController = controller;
	mRootStateMachine->Init(controller);
}

void AnimatorLayer::Animate()
{
	mCrntState->Animate();
	if (mSyncState) {
		float distance = fabs(mCrntState->GetLength() - mSyncState->GetLength());
		if (fabs(distance) <= 0.05f) {
			SyncComplete();
		}
	}

	if (mNextStates.empty()) {
		return;
	}

	sptr<AnimatorMotion> lastState = mNextStates.back();
	for (auto& nextState : mNextStates) {
		nextState->Animate();
	}

	std::erase_if(mNextStates, [](const auto& state) {
		if (state->IsEndAnimation()) {
			state->Reset();
			return true;
		}
		return false;
		});

	if (lastState) {
		if (lastState->IsEndAnimation()) {
			ChangeState(lastState);
		}
		else {
			constexpr float kMaxDuration = .25f;
			const float crntDuration = lastState->GetLength();

			const float t = crntDuration / kMaxDuration;
			if (t < 1.f) {
				mCrntState->SetWeight(1 - t);
				const float t2 = t / mNextStates.size();
				for (auto& nextState : mNextStates) {
					nextState->SetWeight(t2);
				}
			}
			else {
				ChangeState(lastState);
			}
		}
	}
}


void AnimatorLayer::CheckTransition(const AnimatorController* controller)
{
	const auto& nextState = mRootStateMachine->CheckTransition(controller);
	auto it = std::find_if(mNextStates.begin(), mNextStates.end(), [&](sptr<AnimatorMotion> motion) { return motion == nextState; });
	if (it != mNextStates.end()) {
		return;
	}

	if (nextState == mCrntState) {
		if (mNextStates.empty()) {
			return;
		}
		else {
			auto& lastState = mNextStates.back();
			lastState->Reverse();
		}
	}
	else if (nextState != nullptr) {
		nextState->Reset();
		if (mIsSyncSM) {	// 동일한 StateMachine 내에서는 다음 state가 length를 이어받는다.
			if (mCrntState->IsSameStateMachine(nextState)) {
				nextState->SetLength(mCrntState->GetLength());
			}
		}
		
		mNextStates.push_back(nextState);
	}
}

void AnimatorLayer::ChangeState(rsptr<AnimatorMotion> state)
{
	if (!state) {
		return;
	}

	if (!state->IsReverse()) {
		mCrntState->Reset();
		state->SetWeight(1);
		mCrntState = state;
	}
	else {
		mCrntState->SetWeight(1);
		state->Reset();
	}
	mNextStates.clear();

	if (mController) {
		mController->SyncAnimation();
	}
}

void AnimatorLayer::SyncAnimation(rsptr<const AnimatorMotion> srcState)
{
	if (!mNextStates.empty()) {
		return;
	}

	mSyncState = srcState;

	float length = mSyncState->GetLength();
	float threshold = mSyncState->GetMaxLength();
	float myLength = mCrntState->GetLength();
	float distance = length - myLength;
	if (fabs(distance) < 0.05f) {
		SyncComplete();
		return;
	}

	float myThreshold = mCrntState->GetMaxLength();
	threshold = threshold < myThreshold ? threshold : myThreshold;

	float speed{};

	float otherDstTime = threshold - length;	// 대상의 애니메이션 종료 시간
	float myDstTime = threshold - myLength;		// 나의         "
	// 남은 길이가 절반 이상이라면
	if (fabs(distance) > threshold / 2) {
		speed = otherDstTime / myDstTime;
	}
	else {
		// 내 애니메이션이 빠를 경우 천천히,
		//				  느릴 경우 빨리
		// 재생해 두 애니메이션 속도를 맞춘다.
		speed = myDstTime / otherDstTime;
	}

	speed = std::clamp(speed, 0.75f, 1.25f);

	mCrntState->SetSpeed(speed);
}

void AnimatorLayer::SyncComplete()
{
	mCrntState->SetSpeed(1);
	float distance = mCrntState->GetLength() - mSyncState->GetLength();
	if (distance > 0) {
		int a = 5;
	}
	mCrntState->SetLength(mSyncState->GetLength());
	mSyncState = nullptr;
}
#pragma endregion