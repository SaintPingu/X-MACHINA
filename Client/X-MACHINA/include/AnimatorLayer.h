#pragma once
#include "HumanBone.h"

class AnimatorState;
class AnimatorStateMachine;
class AnimatorController;

class AnimatorLayer {
private:
	bool mIsSyncSM{};
	sptr<const AnimatorState> mSyncState{};

	std::string mName{};
	HumanBone mBoneMask{};

	AnimatorController* mController{};
	sptr<AnimatorStateMachine> mRootStateMachine{};

	sptr<AnimatorState>					mCrntState{};
	std::vector<sptr<AnimatorState>>	mNextStates{};

public:
	AnimatorLayer(const std::string& name, sptr<AnimatorStateMachine> rootStateMachine, HumanBone boneMask = HumanBone::None);
	AnimatorLayer(const AnimatorLayer& other);
	virtual ~AnimatorLayer() = default;

	std::string GetName() const { return mName; }
	sptr<const AnimatorState> GetSyncState() const { return mNextStates.empty() ? mCrntState : mNextStates.back(); }
	Vec4x4 GetTransform(int boneIndex, HumanBone boneType) const;

	void SetController(AnimatorController* controller) { mController = controller; }
	void SetCrntStateLength(float length) const;
	void SetSyncStateMachine(bool val) { mIsSyncSM = val; }

public:
	bool CheckBoneMask(HumanBone boneType) { return boneType == HumanBone::None ? true : mBoneMask & boneType; }

	void Animate();

	void CheckTransition(const AnimatorController* controller);

	void SyncAnimation(rsptr<const AnimatorState> srcState);

private:
	void SyncComplete();
	void ChangeState(rsptr<AnimatorState> state);
};