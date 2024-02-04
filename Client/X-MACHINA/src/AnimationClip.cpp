#include "stdafx.h"
#include "AnimationClip.h"


AnimationClip::AnimationClip(float length, int frameRate, int keyFrameCnt, int transformCnt, const std::string& name)
	:
	mLength(length),
	mFrameRate(frameRate),
	mTransformCnt(transformCnt),
	mName(name),
	mKeyFrameTimes(keyFrameCnt),
	mKeyFrameTransforms(keyFrameCnt)
{
	for (auto& transforms : mKeyFrameTransforms) {
		transforms.resize(transformCnt);
	}
}

Vec4x4 AnimationClip::GetSRT(int boneIndex, float position) const
{
	Vec4x4 result = Matrix4x4::Identity();

	const size_t keyFrameCnt = mKeyFrameTimes.size();
	for (size_t i = 0; i < (keyFrameCnt - 1); ++i) {
		if ((mKeyFrameTimes[i] <= position) && (position < mKeyFrameTimes[i + 1])) {
			const float t = (position - mKeyFrameTimes[i]) / (mKeyFrameTimes[i + 1] - mKeyFrameTimes[i]);
			result = Matrix4x4::Interpolate(mKeyFrameTransforms[i][boneIndex], mKeyFrameTransforms[i + 1][boneIndex], t);
			break;
		}
	}

	if (position >= mKeyFrameTimes[keyFrameCnt - 1]) {
		result = mKeyFrameTransforms[keyFrameCnt - 1][boneIndex];
	}

	return result;
}
