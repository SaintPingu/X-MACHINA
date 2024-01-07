#pragma once

class AnimationTrack;
class AnimationSet;
class GameObject;
class SkinMesh;

#include "AnimationClip.h"
#include "AnimationTrack.h"

struct AnimationLoadInfo {

	GameObject*							Model{};
	std::vector<sptr<SkinMesh>>			SkinMeshes{}; //[SkinMeshes], Skinned Mesh Cache
	std::vector<sptr<AnimationClip>>	AnimationClips{};

	void PrepareSkinning();
};

class AnimationController {
public:
	AnimationController(int animationTrackCount, rsptr<AnimationLoadInfo> animationInfo);
	~AnimationController();

public:
	float 							m_fTime = 0.0f;
	std::vector<AnimationTrack> mAnimationTracks{};

	std::vector<sptr<AnimationClip>> mAnimationClips{};
	std::vector<Transform*> mAninmatedBoneFrames{}; //[m_nAnimatedBoneFrames]

	std::vector<sptr<SkinMesh>> mSkinMeshes{}; //[SkinMeshes], Skinned Mesh Cache

	std::vector<ComPtr<ID3D12Resource>> mCB_BoneTransforms{}; //[SkinMeshes]
	std::vector<Vec4x4*> mCBMap_BoneTransforms{}; //[SkinMeshes]

public:
	void UpdateShaderVariables();

	void SetTrackAnimationClip(int trackIndex, int clipIndex);

	void SetTrackEnable(int trackIndex, bool isEnable);
	void SetTrackPosition(int trackIndex, float position);
	void SetTrackSpeed(int trackIndex, float speed);
	void SetTrackWeight(int trackIndex, float weight);

	void AdvanceTime();

	sptr<AnimationClip> GetClip(int trackIndex)
	{
		return mAnimationClips[mAnimationTracks[trackIndex].mClipIndex];
	}

	sptr<AnimationClip> GetClip(const AnimationTrack& track)
	{
		return mAnimationClips[track.mClipIndex];
	}
};

