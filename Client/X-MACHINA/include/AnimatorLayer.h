#pragma once
#include "HumanBone.h"

class AnimatorState;
class AnimatorStateMachine;
class AnimatorController;

class AnimatorLayer {
private:
	sptr<const AnimatorState> mSyncState{};

	std::string mName{};
	HumanBone mBoneMask{};

	AnimatorController* mController{};
	sptr<AnimatorStateMachine> mRootStateMachine{};

	sptr<AnimatorState>	mCrntState{};
	sptr<AnimatorState>	mNextState{};

public:
	AnimatorLayer(const std::string& name, sptr<AnimatorStateMachine> rootStateMachine, HumanBone boneMask = HumanBone::None);
	AnimatorLayer(const AnimatorLayer& other);
	virtual ~AnimatorLayer() = default;

	sptr<const AnimatorState> GetSyncState() const { return mNextState ? mCrntState : mCrntState; }
	Vec4x4 GetTransform(int boneIndex, HumanBone boneType) const;

	void SetController(AnimatorController* controller) { mController = controller; }
	void SetCrntStateLength(float length) const;

public:
	bool CheckBoneMask(HumanBone boneType) { return boneType == HumanBone::None ? true : mBoneMask & boneType; }

	void Animate();

	void CheckTransition(const AnimatorController* controller);
	void ChangeToNextState();

	void SyncAnimation(rsptr<const AnimatorState> srcState);

private:
	void SyncComplete();
};