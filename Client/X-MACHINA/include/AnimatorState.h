#pragma once

struct AnimatorTransition;
class AnimationClip;
class AnimatorStateMachine;
 
// AnimationClip의 재생을 관리한다.
class AnimatorState {
private:
	std::string mName{};

	const AnimatorStateMachine* mStateMachine{};
	sptr<const AnimationClip> mClip{};

	std::vector<sptr<const AnimatorTransition>> mTransitions{};

	int		mIsReverse  = 1;
	float 	mSpeed      = 1.f;
	float 	mCrntLength = 0.f;
	float 	mWeight     = 1.f;

public:
	AnimatorState(rsptr<const AnimatorStateMachine> staeMachine, rsptr<const AnimationClip> clip, const std::vector<sptr<const AnimatorTransition>>& transitions);
	AnimatorState(const AnimatorState& other);
	virtual ~AnimatorState() = default;

	Vec4x4 GetSRT(int boneIndex) const;
	float GetLength() const { return mCrntLength; }
	float GetWeight() const { return mWeight; }
	rsptr<const AnimationClip> GetClip() const { return mClip; }
	std::string GetName() const { return mName; }

	void SetLength(float length);
	void SetSpeed(float speed) { mSpeed = speed; }
	void SetWeight(float weight) { mWeight = weight; }

public:
	void Init();
	bool Animate();
	void Reverse() { mIsReverse *= -1; }

	bool IsSameStateMachine(rsptr<const AnimatorState> other) const;
	bool IsReverse() const { return mIsReverse == -1 ? true : false; }
	bool IsEndAnimation() const;
};