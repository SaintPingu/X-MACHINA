#pragma once

class AnimationTrack;
class AnimationSet;
class GameObject;
class SkinMesh;

#include "AnimationClip.h"
#include "AnimationTrack.h"

struct AnimationLoadInfo {

	GameObject*							mModel{};
	std::vector<sptr<SkinMesh>>			mSkinMeshes{}; //[SkinMeshes], Skinned Mesh Cache
	std::vector<sptr<AnimationClip>>	mAnimationClips{};
};

class AnimationController {
public:
	AnimationController(int animationTrackCount, AnimationLoadInfo* model);
	~AnimationController();

public:
	float 							m_fTime = 0.0f;
	std::vector<AnimationTrack> mAnimationTracks{};

	std::vector<sptr<AnimationClip>> mAnimationClips{};
	std::vector<Transform*> mBoneFrames{}; //[m_nAnimatedBoneFrames]

	std::vector<sptr<SkinMesh>> mSkinMeshes{}; //[SkinMeshes], Skinned Mesh Cache

	std::vector<ComPtr<ID3D12Resource>> mBoneTransforms{}; //[SkinMeshes]
	std::vector<Vec4x4*> mMap_BoneTransforms{}; //[SkinMeshes]

public:
	void UpdateShaderVariables();

	void SetTrackAnimationClip(int trackIndex, int clipIndex);

	void SetTrackEnable(int trackIndex, bool isEnable);
	void SetTrackPosition(int trackIndex, float position);
	void SetTrackSpeed(int trackIndex, float speed);
	void SetTrackWeight(int trackIndex, float weight);

	void AdvanceTime(GameObject* model);

	sptr<AnimationClip> GetClip(int trackIndex)
	{
		return mAnimationClips[mAnimationTracks[trackIndex].mClipIndex];
	}

	sptr<AnimationClip> GetClip(const AnimationTrack& track)
	{
		return mAnimationClips[track.mClipIndex];
	}
};

