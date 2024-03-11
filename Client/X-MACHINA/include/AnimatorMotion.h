#pragma once

struct AnimatorTransition;
struct AnimatorParameter;
class AnimatorController;
class AnimationClip;
class AnimatorStateMachine;
 

// AnimatorState�� BlendTree�� �θ� Ŭ����
// �ִϸ��̼� StateMachine���� �� animation state�� ���� ���¸� ��Ÿ����.
class AnimatorMotion abstract {
private:

	float 	mSpeed      = 1.f;
	float 	mCrntLength = 0.f;
	float	mMaxLength  = 0.f;
	float 	mWeight     = 1.f;

	int		mIsReverse = 1;

	std::string mName{};
	const AnimatorStateMachine* mStateMachine{};
	std::vector<sptr<const AnimatorTransition>> mTransitions{};

public:
	AnimatorMotion(rsptr<const AnimatorStateMachine> stateMachine, const std::vector<sptr<const AnimatorTransition>>& transitions, const std::string& name, float maxLength);
	AnimatorMotion(const AnimatorMotion& other);
	virtual ~AnimatorMotion() = default;

	virtual Vec4x4 GetSRT(int boneIndex) const abstract;
	std::string GetName() const { return mName; }
	float GetLength() const { return mCrntLength; }
	float GetMaxLength() const { return mMaxLength; }
	float GetWeight() const { return mWeight; }

	void ResetLength();
	void SetLength(float length);
	void SetSpeed(float speed) { mSpeed = speed; }
	void SetWeight(float weight) { mWeight = weight; }

public:
	virtual void Init(const AnimatorController* controller) {};
	void Reset();

	void Reverse() { mIsReverse *= -1; }

	bool IsEndAnimation() const;
	bool IsSameStateMachine(rsptr<const AnimatorMotion> other) const;
	bool IsReverse() const { return mIsReverse == -1 ? true : false; }

	virtual bool Animate();
};


// AnimationClip�� ����� �����Ѵ�.
class AnimatorTrack {
private:
	sptr<const AnimationClip> mClip{};

public:
	AnimatorTrack(rsptr<const AnimationClip> clip);
	AnimatorTrack(const AnimatorTrack& other);
	virtual ~AnimatorTrack() = default;

	rsptr<const AnimationClip> GetClip() const { return mClip; }

protected:
	Vec4x4 GetSRT(int boneIndex, float length) const;
};


// AnimationClip�� ���¸� �����Ѵ�.
class AnimatorState : public AnimatorMotion, public AnimatorTrack {
	using base = AnimatorMotion;

public:
	AnimatorState(rsptr<const AnimatorStateMachine> stateMachine, const std::vector<sptr<const AnimatorTransition>>& transitions, rsptr<const AnimationClip> clip);
	AnimatorState(const AnimatorState& other);
	virtual ~AnimatorState() = default;

	virtual Vec4x4 GetSRT(int boneIndex) const override;
};



// BlendTree���� ����ϴ� ���� Motion
class ChildMotion : public AnimatorTrack {
private:
	float	mCrntLength = 0.f;
	float 	mWeight     = 1.f;

	Vec2	mPosition{};

public:
	ChildMotion(rsptr<const AnimationClip> clip, const Vec2& position);
	ChildMotion(const ChildMotion& other);
	virtual ~ChildMotion() = default;

	float GetLength() const { return mCrntLength; }
	float GetWeight() const { return mWeight; }
	Vec2 GetPosition() const { return mPosition; }
	Vec4x4 GetSRT(int boneIndex) const;

	void SetLength(float length) { mCrntLength = length; }
	void SetWeight(float weight) { mWeight = weight; }
};


// ���� Motion�� �����Ͽ� ����Ѵ�.
class BlendTree : public AnimatorMotion {
	using base = AnimatorMotion;

private:
	std::vector<sptr<ChildMotion>> mMotions{};

	const AnimatorParameter* x{};
	const AnimatorParameter* y{};

	void CalculateWeights() const;

public:
	BlendTree(rsptr<const AnimatorStateMachine> stateMachine, const std::vector<sptr<const AnimatorTransition>>& transitions, const std::string& name, std::vector<sptr<ChildMotion>> motions);
	BlendTree(const BlendTree& other);
	virtual ~BlendTree() = default;

	virtual Vec4x4 GetSRT(int boneIndex) const override;

public:
	virtual void Init(const AnimatorController* controller) override;

	virtual bool Animate() override;

};