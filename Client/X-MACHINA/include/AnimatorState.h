#pragma once

class AnimationClip;
class AnimatorState;

// AnimatorState�� ���� ���� ����
struct AnimationCondition {
	std::string mode{};
	std::string param{};
	float threshold{};
};

// AnimatorState�� ���� ���̸� �����Ѵ�.
struct AnimatorTransition {
	std::string Destination{};
	std::vector<AnimationCondition> Conditions{};
};

// AnimationClip�� ����� �����Ѵ�.
class AnimatorState {
private:
	std::string mName{};
	sptr<const AnimationClip> mClip{};
	AnimatorState* mNextTrack{};

	float 	mSpeed     = 1.f;
	float 	mCrntLength  = 0.0f;
	float 	mWeight    = 1.0f;

	std::vector<AnimatorTransition> mTransitions{};

public:
	AnimatorState(const std::string name, rsptr<const AnimationClip> clip, const std::vector<AnimatorTransition>& transitions);
	~AnimatorState() = default;

	Vec4x4 GetSRT(int boneIndex) const;
	float GetCrntLength() const { return mCrntLength; }
	float GetWeight() const { return mWeight; }
	rsptr<const AnimationClip> GetClip() const { return mClip; }

	void SetSpeed(float speed) { mSpeed = speed; }
	void SetWeight(float weight) { mWeight = weight; }

public:
	void Init();
	int Animate();

private:
	bool IsEndAnimation();
};