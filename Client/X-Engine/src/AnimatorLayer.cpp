#include "EnginePch.h"
#include "AnimatorLayer.h"

#include "Timer.h"
#include "AnimationClip.h"
#include "AnimatorController.h"
#include "AnimatorMotion.h"
#include "AnimatorStateMachine.h"


#pragma region AnimatorLayer
AnimatorLayer::AnimatorLayer(const std::string& name, rsptr<AnimatorStateMachine> rootStateMachine, HumanBone boneMask)
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

Matrix AnimatorLayer::GetTransform(int boneIndex, HumanBone boneType) const
{
	Matrix transform = mCrntState->GetSRT(boneIndex) * mCrntState->GetWeight();
	for (auto& nextState : mNextStates) {
		transform += nextState->GetSRT(boneIndex) * nextState->GetWeight();
	}

	return transform;
}

void AnimatorLayer::Init(AnimatorController* controller)
{
	mController = controller;
	mRootStateMachine->Init(controller, this);
	CheckTransition(controller);
}

void AnimatorLayer::Release()
{
	mRootStateMachine->Release();
	mRootStateMachine = nullptr;
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

	AnimatorMotion* nextState = mNextStates.front();
	for (auto& nextState : mNextStates) {
		nextState->Animate();
	}

	if (!mIsReverse) {
		mCrntState->DecWeight();

		if (mCrntState->GetWeight() <= 0.f) {
			ChangeState(nextState);
			if (!mNextStates.empty()) {
				mCrntState->DecWeight();
			}
		}
	}
	else {
		mCrntState->IncWeight();

		if (mCrntState->GetWeight() >= 1.f) {
			mIsReverse = false;
		}
	}

	if (mNextStates.empty()) {
		return;
	}

	float totalWeight = mCrntState->GetWeight();
	for (auto& nextState : mNextStates) {
		if (nextState != mNextStates.back()) {
			nextState->DecWeight();
			float weight = nextState->GetWeight();
			if (weight > 0.f) {
				totalWeight += weight;
			}
		}
	}

	mNextStates.back()->SetWeight(1.f - totalWeight);

	std::erase_if(mNextStates, [](const auto& state) {
		if (state->GetWeight() <= 0.f) {
			state->StopAnimation();
			return true;
		}
		return false;
		});

	if (mNextStates.empty()) {
		mCrntState->SetWeight(1);
	}
}


AnimatorMotion* AnimatorLayer::CheckTransition(const AnimatorController* controller, bool isChangeImmed)
{
	const auto& nextState = mRootStateMachine->CheckTransition(controller);
	if (!nextState) {
		return nullptr;
	}

	if (isChangeImmed) {
		mNextStates.clear();
		ChangeState(nextState);
		return nextState;
	}

	if (PushState(nextState)) {
		return nextState;
	}

	return nullptr;
}

void AnimatorLayer::ChangeState(AnimatorMotion* state)
{
	if (!state) {
		return;
	}

	mCrntState->CallbackChange();
	mCrntState->Reset();
	mCrntState = state;

	for (auto it = mNextStates.begin(); it != mNextStates.end(); ++it) {
		if (*it == state) {
			mNextStates.erase(it);
			break;
		}
	}

	if (mNextStates.empty()) {
		mCrntState->SetWeight(1);
	}

	if (mController) {
		mController->SyncAnimation();
	}
}

void AnimatorLayer::SyncAnimation(const AnimatorMotion* srcState)
{
	if (!mIsSyncSM || !mNextStates.empty() || !mCrntState->IsActiveSync()) {
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

AnimatorMotion* AnimatorLayer::FindMotionByName(const std::string& motionName) const
{
	return mRootStateMachine->FindMotionByName(motionName);
}

void AnimatorLayer::SyncComplete()
{
	mCrntState->ResetSpeed();
	float distance = mCrntState->GetLength() - mSyncState->GetLength();
	if (distance > 0) {
		int a = 5;
	}
	mCrntState->SetLength(mSyncState->GetLength());
	mSyncState = nullptr;
}
#pragma endregion

bool AnimatorLayer::PushState(AnimatorMotion* nextState)
{
	auto it = std::find_if(mNextStates.begin(), mNextStates.end(), [&](AnimatorMotion* motion) { return motion == nextState; });
	if (it != mNextStates.end()) {
		return false;
	}

	if (nextState == mCrntState) {
		if (mNextStates.empty()) {
			return false;
		}
		else {
			mIsReverse = true;
		}
	}
	else if (nextState != nullptr) {
		nextState->Reset();
		if (mIsSyncSM) {	// 동일한 StateMachine 내에서는 다음 state가 length를 이어받는다.
			if (mCrntState->IsSameStateMachine(nextState)) {
				nextState->SetLength(mCrntState->GetLength());
			}
		}

		nextState->SetWeight(0);
		if (mIsReverse) {
			mNextStates.insert(mNextStates.begin(), nextState);
		}
		else {
			mNextStates.push_back(nextState);
		}
	}

	return true;
}

bool AnimatorLayer::SetAnimation(const std::string& motionName)
{
	const auto& state = mRootStateMachine->FindMotionByName(motionName);
	if (state) {
		PushState(state);
		return true;
	}

	return false;
}

void AnimatorLayer::AddStates(int& index, std::unordered_map<int, std::string>& motionMapInt, std::unordered_map<std::string, int>& motionMapString)
{
	mRootStateMachine->AddStates(index, motionMapInt, motionMapString);
}
