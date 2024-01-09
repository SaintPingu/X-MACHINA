#pragma once

// AnimationClip의 재생을 관리한다.
class AnimationTrack {
private:
	bool 	mIsEnable  = true;
	float 	mSpeed     = 3.f;
	float 	mPosition  = 0.0f;
	float 	mWeight    = 1.0f;
	int 	mClipIndex = 0;

public:
	AnimationTrack() = default;
	~AnimationTrack() = default;

	float GetPosition() const { return mPosition; }
	float GetWeight() const { return mWeight; }
	int GetClipIndex() const { return mClipIndex; }
	bool IsEnable() const { return mIsEnable; }

	void SetClip(int clipIndex) { mClipIndex = clipIndex; }
	void SetEnable(bool isEnable) { mIsEnable = isEnable; }
	void SetSpeed(float speed) { mSpeed = speed; }
	void SetWeight(float weight) { mWeight = weight; }

public:
	void Init();

	void Animate(float maxLength);
};

