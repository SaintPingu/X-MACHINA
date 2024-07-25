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
	const AnimatorMotion* mSyncState{};

	std::string mName{};
	HumanBone mBoneMask{};

	AnimatorController* mController{};
	sptr<AnimatorStateMachine> mRootStateMachine{};

	AnimatorMotion*				mCrntState{};
	std::vector<AnimatorMotion*>	mNextStates{};

public:
	AnimatorLayer(const std::string& name, rsptr<AnimatorStateMachine> rootStateMachine, HumanBone boneMask = HumanBone::None);
	AnimatorLayer(const AnimatorLayer& other);
	virtual ~AnimatorLayer() = default;

	std::string GetName() const { return mName; }
	const AnimatorMotion* GetSyncState() const { return GetLastMotion(); }
	AnimatorMotion* GetLastMotion() const		{ return mNextStates.empty() ? mCrntState : mNextStates.back(); }
	Matrix GetTransform(const std::string& boneName, HumanBone boneType) const;
	AnimatorController* GetController() const { return mController; }

	void SetSyncStateMachine(bool val) { mIsSyncSM = val; }

public:
	void Init(AnimatorController* controller);
	void Release();

	bool CheckBoneMask(HumanBone boneType) { return boneType == HumanBone::None ? true : mBoneMask & boneType; }

	void Animate();

	AnimatorMotion* CheckTransition(const AnimatorController* controller, bool isChangeImmed = false);
	bool IsEndTransition() const { return mNextStates.empty(); }

	void SyncAnimation(const AnimatorMotion* srcState);

	AnimatorMotion* FindMotionByName(const std::string& motionName) const;
	AnimatorMotion* GetCrntMotion() const { return mCrntState; }

	bool PushState(AnimatorMotion* nextState);

	bool SetAnimation(const std::string& motionName);
	void AddStates(int& index, std::unordered_map<int, std::string>& motionMapInt, std::unordered_map<std::string, int>& motionMapString);

private:
	void SyncComplete();
	void ChangeState(AnimatorMotion* state);
};