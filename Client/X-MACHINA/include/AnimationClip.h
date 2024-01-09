#pragma once

class Transform;

class AnimationClip {

public:
	AnimationClip(float length, int frameRate, int keyFrameCnt, int boneCnt, const std::string& name);
	~AnimationClip() = default;

public:
	std::string mName;

	float	mLength = 0.0f;
	int		mFrameRate = 0; //m_fTicksPerSecond
	int		mTransformCnt = 0;

	std::vector<float> mKeyFrameTimes{};
	std::vector<std::vector<Vec4x4>> mKeyFrameTransforms{};

	Vec4x4 GetSRT(int boneIndex, float position) const;
};