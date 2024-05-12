#pragma once
#include "HumanBone.h"

class AnimatorMotion;
class AnimatorStateMachine;
class AnimatorController;
class HumanBone;

class AnimatorLayer {
private:
	bool mIsReverse{};
	bool mIsSyncSM{};
	sptr<const AnimatorMotion> mSyncState{};

	std::string mName{};
	HumanBone mBoneMask{};

	AnimatorController* mController{};
	sptr<AnimatorStateMachine> mRootStateMachine{};

	sptr<AnimatorMotion>				mCrntState{};
	std::vector<sptr<AnimatorMotion>>	mNextStates{};

public:
	AnimatorLayer(const std::string& name, sptr<AnimatorStateMachine> rootStateMachine, HumanBone boneMask = HumanBone::None);
	AnimatorLayer(const AnimatorLayer& other);
	virtual ~AnimatorLayer() = default;

	std::string GetName() const { return mName; }
	sptr<const AnimatorMotion> GetSyncState() const { return GetLastMotion(); }
	sptr<AnimatorMotion> GetLastMotion() const		{ return mNextStates.empty() ? mCrntState : mNextStates.back(); }
	Matrix GetTransform(int boneIndex, HumanBone boneType) const;
	AnimatorController* GetController() const { return mController; }

	void SetSyncStateMachine(bool val) { mIsSyncSM = val; }

public:
	void Init(AnimatorController* controller);

	bool CheckBoneMask(HumanBone boneType) { return boneType == HumanBone::None ? true : mBoneMask & boneType; }

	void Animate();

	sptr<AnimatorMotion> CheckTransition(const AnimatorController* controller, bool isChangeImmed = false);
	bool IsEndTransition() const { return mNextStates.empty(); }

	void SyncAnimation(rsptr<const AnimatorMotion> srcState);

	sptr<AnimatorMotion> FindMotionByName(const std::string& motionName) const;
	sptr<AnimatorMotion> GetCrntMotion() const { return mCrntState; }

	void PushState(rsptr<AnimatorMotion> nextState);

	bool SetAnimation(const std::string& motionName);
	void AddStates(int& index, std::unordered_map<int, std::string>& motionMapInt, std::unordered_map<std::string, int>& motionMapString);

private:
	void SyncComplete();
	void ChangeState(rsptr<AnimatorMotion> state);
};