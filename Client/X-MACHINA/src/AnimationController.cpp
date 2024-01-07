#include "stdafx.h"
#include "AnimationController.h"
#include "DXGIMgr.h"

#include "Mesh.h"
#include "Timer.h"
#include "Transform.h"

#include "Object.h"

void AnimationLoadInfo::PrepareSkinning()
{
	for (auto& skinMesh : SkinMeshes) {
		skinMesh->PrepareSkinning(Model);
	}
}

AnimationController::AnimationController(int animationTrackCount, rsptr<AnimationLoadInfo> animationInfo)
{
	mAnimationTracks.resize(animationTrackCount);

	mAnimationClips = animationInfo->AnimationClips;

	mSkinMeshes = animationInfo->SkinMeshes;
	const size_t skinMeshCount = mSkinMeshes.size();

	// 각 SkinMesh에 대한 CB생성
	mCB_BoneTransforms.resize(skinMeshCount);
	mCBMap_BoneTransforms.resize(skinMeshCount);

	size_t byteSize = (((sizeof(Vec4x4) * gkSkinBoneSize) + 255) & ~255); //256의 배수
	for (size_t i = 0; i < skinMeshCount; ++i)
	{
		D3DUtil::CreateBufferResource(nullptr, byteSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, mCB_BoneTransforms[i]);
		mCB_BoneTransforms[i]->Map(0, NULL, (void**)&mCBMap_BoneTransforms[i]);
	}
}

AnimationController::~AnimationController()
{
	for (size_t i = 0; i < mSkinMeshes.size(); ++i)
	{
		mCB_BoneTransforms[i]->Unmap(0, NULL);
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
		mSkinMeshes[i]->mCB_BoneTransforms = mCB_BoneTransforms[i];
		mSkinMeshes[i]->mCBMap_BoneTransforms = mCBMap_BoneTransforms[i];
	}
}

void AnimationController::AdvanceTime()
{
	m_fTime += DeltaTime();
	for(const auto& track : mAnimationTracks) {
		GetClip(track)->UpdatePosition(DeltaTime() * track.mSpeed);
	}

	auto& boneFrames = mSkinMeshes.front()->mBoneFrames;

	for (int j = 0; j < boneFrames.size(); j++)
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

		boneFrames[j]->SetLocalTransform(transform);
	}
}