#pragma once
#include "HumanBone.h"

class AnimatorState;
class AnimatorStateMachine;
class AnimatorController;

class AnimatorLayer {
private:
	std::string mName;
	HumanBone mBoneMask;

	sptr<AnimatorStateMachine> mRootStateMachine;

	sptr<AnimatorState>	mCrntState{};
	sptr<AnimatorState>	mNextState{};

public:
	AnimatorLayer(const std::string& name, sptr<AnimatorStateMachine> rootStateMachine, HumanBone boneMask = HumanBone::None);
	AnimatorLayer(const AnimatorLayer& other);
	virtual ~AnimatorLayer() = default;

	Vec4x4 GetTransform(int boneIndex, HumanBone boneType) const;

public:
	bool CheckBoneMask(HumanBone boneType) { return boneType == HumanBone::None ? true : mBoneMask & boneType; }

	void Animate();

	void CheckTransition(const AnimatorController* controller);
	void ChangeToNextState();
};