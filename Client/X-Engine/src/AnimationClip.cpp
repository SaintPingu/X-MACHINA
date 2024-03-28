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
	for (auto& transforms : mKeyFrameTransforms) {
		transforms.resize(transformCnt);
	}
}

Matrix AnimationClip::GetSRT(int boneIndex, float position) const
{

	const float lastPosition = mKeyFrameTimes.back();
	if (position >= lastPosition) {
		const size_t lastIndex = mKeyFrameTimes.size() - 1;
		return mKeyFrameTransforms[lastIndex][boneIndex];
	}
	else if (position < 0.f) {
		position = 0.f;
	}

	auto keyFramePosition = std::lower_bound(mKeyFrameTimes.begin(), mKeyFrameTimes.end(), position);
	if (keyFramePosition != mKeyFrameTimes.begin()) {
		keyFramePosition = std::prev(keyFramePosition);
	}

	const size_t keyFrameIndex = std::distance(mKeyFrameTimes.begin(), keyFramePosition);
	const float t = (position - mKeyFrameTimes[keyFrameIndex]) / (mKeyFrameTimes[keyFrameIndex + 1] - mKeyFrameTimes[keyFrameIndex]);

	return Matrix4x4::Interpolate(mKeyFrameTransforms[keyFrameIndex][boneIndex], mKeyFrameTransforms[keyFrameIndex + 1][boneIndex], t);
}
