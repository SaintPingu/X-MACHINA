#pragma once
class AnimationTrack {
public:
	AnimationTrack() = default;
	~AnimationTrack() = default;

public:
	bool 							mIsEnable       = true;
	float 							mSpeed          = 1.0f;
	float 							mPosition       = 0.0f;
	float 							mWeight         = 1.0f;
	int 							mClipIndex = 2;

public:
	void SetAnimationSet(int clipIndex) { mClipIndex = clipIndex; }

	void SetEnable(bool isEnable) { mIsEnable = isEnable; }
	void SetSpeed(float speed) { mSpeed = speed; }
	void SetWeight(float weight) { mWeight = weight; }
	void SetPosition(float position) { mPosition = position; }
};

