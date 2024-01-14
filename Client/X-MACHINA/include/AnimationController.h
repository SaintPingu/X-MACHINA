#pragma once

class AnimationTrack;
class AnimationSet;
class GameObject;
class SkinMesh;

#include "AnimationClip.h"
#include "AnimationTrack.h"

// 한 모델(MasterModel)의 파일(정보)을 읽을 때의 Animation 정보
struct AnimationLoadInfo {
	std::vector<sptr<SkinMesh>>				SkinMeshes{};
	std::vector<sptr<const AnimationClip>>	AnimationClips{};
};

// Animation의 재생 및 상태 전이 등의 전반을 관리한다.
class AnimationController {
private:
	std::vector<AnimationTrack>				mAnimationTracks{};
	std::vector<sptr<const AnimationClip>> mAnimationClips{};

	std::vector<std::vector<Transform*>>	mBoneFramesList{};
	std::vector<sptr<SkinMesh>> mSkinMeshes{};

	std::vector<ComPtr<ID3D12Resource>> mCB_BoneTransforms{};
	std::vector<Vec4x4*>				mCBMap_BoneTransforms{};

public:
	AnimationController(int animationTrackCount, rsptr<const AnimationLoadInfo> animationInfo, GameObject* avatar);
	~AnimationController();

public:
	void UpdateShaderVariables();

	void SetTrackAnimationClip(int trackIndex, int clipIndex);

	void SetTrackEnable(int trackIndex, bool isEnable);
	void SetTrackSpeed(int trackIndex, float speed);
	void SetTrackWeight(int trackIndex, float weight);

	void Animate();

private:
	sptr<const AnimationClip> GetClip(int trackIndex) const
	{
		return mAnimationClips[mAnimationTracks[trackIndex].GetClipIndex()];
	}

	sptr<const AnimationClip> GetClip(const AnimationTrack& track) const
	{
		return mAnimationClips[track.GetClipIndex()];
	}
};
