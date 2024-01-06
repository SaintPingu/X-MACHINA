#include "stdafx.h"
#include "AnimationController.h"
#include "DXGIMgr.h"

#include "Mesh.h"
#include "Timer.h"
#include "Transform.h"


AnimationController::AnimationController(int animationTrackCount, AnimationLoadInfo* model)
{
	mAnimationTracks.resize(animationTrackCount);

	mAnimationClips = model->mAnimationClips;

	mSkinMeshes = model->mSkinMeshes;
	const size_t skinMeshCount = mSkinMeshes.size();

	mBoneTransforms.resize(skinMeshCount);
	mMap_BoneTransforms.resize(skinMeshCount);

	size_t byteSize = (((sizeof(Vec4x4) * gkSkinBoneSize) + 255) & ~255); //256ÀÇ ¹è¼ö
	for (size_t i = 0; i < skinMeshCount; ++i)
	{
		D3DUtil::CreateBufferResource(nullptr, byteSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, mBoneTransforms[i]);
		mBoneTransforms[i]->Map(0, NULL, (void**)&mMap_BoneTransforms[i]);
	}
}

AnimationController::~AnimationController()
{
	for (size_t i = 0; i < mSkinMeshes.size(); ++i)
	{
		mBoneTransforms[i]->Unmap(0, NULL);
	}
}

void AnimationController::SetTrackAnimationClip(int trackIndex, int clipIndex)
{
	mAnimationTracks[trackIndex].mClipIndex = clipIndex;
}

void AnimationController::SetTrackEnable(int trackIndex, bool isEnable)
{
	mAnimationTracks[trackIndex].SetEnable(isEnable);
}

void AnimationController::SetTrackPosition(int trackIndex, float position)
{
	mAnimationTracks[trackIndex].SetPosition(position);
	sptr<AnimationClip> clip = GetClip(trackIndex);
	if (clip) {
		clip->mPosition = position;
	}
}

void AnimationController::SetTrackSpeed(int trackIndex, float speed)
{
	mAnimationTracks[trackIndex].SetSpeed(speed);
}

void AnimationController::SetTrackWeight(int trackIndex, float weight)
{
	mAnimationTracks[trackIndex].SetWeight(weight);
}

void AnimationController::UpdateShaderVariables()
{
	for (int i = 0; i < mSkinMeshes.size(); i++)
	{
		mSkinMeshes[i]->mCB_BoneTransforms = mBoneTransforms[i];
		mSkinMeshes[i]->mCBMap_BoneTransforms = mMap_BoneTransforms[i];
	}
}

void AnimationController::AdvanceTime(GameObject* model)
{
	m_fTime += DeltaTime();
	for(const auto& track : mAnimationTracks) {
		GetClip(track)->UpdatePosition(DeltaTime() * track.mSpeed);
	}

	for (int j = 0; j < mBoneFrames.size(); j++)
	{
		Vec4x4 transform = Matrix4x4::Zero();
		for (const auto& track : mAnimationTracks) {
			if (!track.mIsEnable) {
				continue;
			}

			Vec4x4 trackTransform = GetClip(track)->GetSRT(j);
			Vec4x4 trackScale = Matrix4x4::Scale(trackTransform, track.mWeight);
			transform = Matrix4x4::Add(transform, trackScale);
		}

		mBoneFrames[j]->SetLocalTransform(transform);
	}
}