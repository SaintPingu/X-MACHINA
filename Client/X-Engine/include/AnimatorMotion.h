#pragma once

struct AnimatorTransition;
struct AnimatorParameter;
class AnimatorController;
class AnimationClip;
class AnimatorStateMachine;

struct AnimatorMotionInfo {
	std::string Name{};
	float Length{};
	float Speed{};

	sptr<const AnimatorStateMachine> StateMachine;
	std::vector<sptr<const AnimatorTransition>> Transitions{};
};

struct MotionCallback {
	float Time;
	std::function<void()> Callback;
	bool Called;

	void Reset()
	{
		Called = false;
	}
};

// AnimatorState과 BlendTree의 부모 클래스
// 애니메이션 StateMachine에서 각 animation state의 현재 상태를 나타낸다.
class AnimatorMotion abstract {
private:

	const float mkTransitionSpeed = 4.5f;
	const float mkOriginSpeed{};

	float 	mCrntSpeed{};

	float 	mCrntLength{};
	float	mMaxLength{};
	float 	mWeight{};

	int		mIsReverse = 1;

	std::string mName{};
	const AnimatorStateMachine* mStateMachine{};
	std::vector<sptr<const AnimatorTransition>> mTransitions{};

	std::vector<MotionCallback> mCallbackList;

public:
	AnimatorMotion(const AnimatorMotionInfo& info);
	AnimatorMotion(const AnimatorMotion& other);
	virtual ~AnimatorMotion() = default;

	virtual Matrix GetSRT(int boneIndex) const abstract;
	std::string GetName() const { return mName; }
	float GetLength() const { return mCrntLength; }
	float GetMaxLength() const { return mMaxLength; }
	float GetWeight() const { return mWeight; }

	void ResetLength();
	void SetLength(float length);
	void ResetSpeed() { mCrntSpeed = mkOriginSpeed; }
	void SetSpeed(float speed) { mCrntSpeed = speed; }
	void SetWeight(float weight) { mWeight = weight; }

	void AddCallback(const std::function<void()>& callback, int frame);

public:
	virtual void Init(const AnimatorController* controller) {};
	void Reset();

	void Reverse(bool val) { mIsReverse = (val == true) ? -1 : 1; }

	bool IsEndAnimation() const;
	bool IsSameStateMachine(rsptr<const AnimatorMotion> other) const;
	bool IsReverse() const { return mIsReverse == -1 ? true : false; }

	virtual bool Animate();

	void DecWeight();
	void IncWeight();

protected:
	virtual float GetFrameTime(int frame) abstract;
};


// AnimationClip의 재생을 관리한다.
class AnimatorTrack {
private:
	sptr<const AnimationClip> mClip{};

public:
	AnimatorTrack(rsptr<const AnimationClip> clip);
	AnimatorTrack(const AnimatorTrack& other);
	virtual ~AnimatorTrack() = default;

	rsptr<const AnimationClip> GetClip() const { return mClip; }

protected:
	Matrix GetSRT(int boneIndex, float length) const;
};


// AnimationClip의 상태를 관리한다.
class AnimatorState : public AnimatorMotion, public AnimatorTrack {
	using base = AnimatorMotion;

public:
	AnimatorState(const AnimatorMotionInfo& info, rsptr<const AnimationClip> clip);
	AnimatorState(const AnimatorState& other);
	virtual ~AnimatorState() = default;

	virtual Matrix GetSRT(int boneIndex) const override;

protected:
	virtual float GetFrameTime(int frame) override;
};



// BlendTree에서 사용하는 하위 Motion
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
	Matrix GetSRT(int boneIndex) const;

	void SetLength(float length) { mCrntLength = length; }
	void SetWeight(float weight) { mWeight = weight; }
};


// 여러 Motion을 블렌딩하여 재생한다.
class BlendTree : public AnimatorMotion {
	using base = AnimatorMotion;

private:
	std::vector<sptr<ChildMotion>> mMotions{};

	const AnimatorParameter* x{};
	const AnimatorParameter* y{};

	void CalculateWeights() const;

public:
	BlendTree(const AnimatorMotionInfo& info, std::vector<sptr<ChildMotion>> motions);
	BlendTree(const BlendTree& other);
	virtual ~BlendTree() = default;

	virtual Matrix GetSRT(int boneIndex) const override;

public:
	virtual void Init(const AnimatorController* controller) override;

	virtual bool Animate() override;

protected:
	virtual float GetFrameTime(int frame) override;
};