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

	float 	mSpeed      = .5f;
	float 	mCrntLength = 0.f;
	float 	mWeight     = 1.f;

	std::vector<sptr<AnimatorTransition>> mTransitions{};

public:
	AnimatorState(rsptr<const AnimationClip> clip, const std::vector<sptr<AnimatorTransition>>& transitions);
	AnimatorState(const AnimatorState& other);
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
	bool Animate();

	std::string CheckTransition(const std::string& param, float value);

private:
	bool IsEndAnimation();
};