#pragma once

class AnimationClip;
class AnimatorLayer;

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

	std::string CheckTransition(const std::string& param, float value) const;
};
 
// AnimationClip�� ����� �����Ѵ�.
class AnimatorState {
private:
	sptr<const AnimatorLayer> mLayer{};

	std::string mName{};
	sptr<const AnimationClip> mClip{};

	float 	mSpeed      = .5f;
	float 	mCrntLength = 0.f;
	float 	mWeight     = 1.f;

	std::vector<sptr<const AnimatorTransition>> mTransitions{};

public:
	AnimatorState(rsptr<const AnimatorLayer> layer, rsptr<const AnimationClip> clip, const std::vector<sptr<const AnimatorTransition>>& transitions);
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

	sptr<AnimatorState> CheckTransition(const std::string& param, float value);

private:
	bool IsEndAnimation();
};