#pragma once

class AnimationClip;

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

	float 	mSpeed     = 1.f;
	float 	mCrntLength  = 0.0f;
	float 	mWeight    = 1.0f;

	std::vector<AnimatorTransition> mTransitions{};

public:
	AnimatorState(rsptr<const AnimationClip> clip, const std::vector<AnimatorTransition>& transitions);
	~AnimatorState() = default;

	Vec4x4 GetSRT(int boneIndex) const;
	float GetCrntLength() const { return mCrntLength; }
	float GetWeight() const { return mWeight; }
	rsptr<const AnimationClip> GetClip() const { return mClip; }
	std::string GetName() const { return mName; }

	void SetSpeed(float speed) { mSpeed = speed; }
	void SetWeight(float weight) { mWeight = weight; }

public:
	void Init();
	int Animate();

	std::string CheeckTransition(const std::string& param, float value);

private:
	bool IsEndAnimation();
};