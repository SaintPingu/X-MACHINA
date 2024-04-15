#pragma once

#pragma region Include
#include "Resources.h"
#pragma endregion

class Transform;

class AnimationClip : public Resource {
public:
	std::string mName;

	float	mLength = 0.0f;
	int		mFrameRate = 0;
	int		mTransformCnt = 0;

	std::vector<float> mKeyFrameTimes{};
	std::vector<std::vector<Matrix>> mKeyFrameTransforms{};

public:
	AnimationClip(float length, int frameRate, int keyFrameCnt, int boneCnt, const std::string& name);
	~AnimationClip() = default;

	float GetFrameTime(int frame) const { return mKeyFrameTimes[frame]; }
	float GetMaxFrameTime() const { return mKeyFrameTimes.back(); }
	int GetMaxFrameRate() const { return static_cast<int>(mKeyFrameTimes.size()); }

public:
	Matrix GetSRT(int boneIndex, float position) const;
};