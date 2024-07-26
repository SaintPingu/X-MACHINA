#include "EnginePch.h"
#include "AnimationClip.h"


AnimationClip::AnimationClip(float length, int frameRate, int keyFrameCnt, int transformCnt, const std::string& name)
	:
	Resource(ResourceType::AnimationClip),
	mLength(length),
	mFrameRate(frameRate),
	mTransformCnt(transformCnt),
	mName(name),
	mKeyFrameTimes(keyFrameCnt),
	mKeyFrameTransforms(keyFrameCnt)
{

}

Matrix AnimationClip::GetSRT(const std::string& boneName, float position) const
{

	const float lastPosition = mKeyFrameTimes.back();
	if (position >= lastPosition) {
		const int lastIndex = static_cast<int>(mKeyFrameTimes.size() - 1);
		return GetMatrix(boneName, lastIndex);
	}
	else if (position < 0.f) {
		position = 0.f;
	}

	auto keyFramePosition = std::lower_bound(mKeyFrameTimes.begin(), mKeyFrameTimes.end(), position);
	if (keyFramePosition != mKeyFrameTimes.begin()) {
		keyFramePosition = std::prev(keyFramePosition);
	}

	const int keyFrameIndex = static_cast<int>(std::distance(mKeyFrameTimes.begin(), keyFramePosition));
	const float t = (position - mKeyFrameTimes[keyFrameIndex]) / (mKeyFrameTimes[keyFrameIndex + 1] - mKeyFrameTimes[keyFrameIndex]);

	return Matrix4x4::Interpolate(GetMatrix(boneName, keyFrameIndex), GetMatrix(boneName, keyFrameIndex + 1), t);
}

const Matrix& AnimationClip::GetMatrix(const std::string& boneName, int keyFrameIdx) const
{
	return (mKeyFrameTransforms.at(boneName))[keyFrameIdx];
}
