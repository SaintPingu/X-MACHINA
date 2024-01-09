#include "stdafx.h"
#include "AnimationController.h"
#include "DXGIMgr.h"

#include "Mesh.h"
#include "Timer.h"
#include "Transform.h"

#include "Object.h"


AnimationController::AnimationController(int animationTrackCount, rsptr<const AnimationLoadInfo> animationInfo, GameObject* avatar)
{
	mAnimationTracks.resize(animationTrackCount);

	mAnimationClips = animationInfo->AnimationClips;

	mSkinMeshes = animationInfo->SkinMeshes;
	for (auto& skinMesh : mSkinMeshes) {
		skinMesh->PrepareSkinning(avatar);
	}
	const size_t skinMeshCount = mSkinMeshes.size();

	// 각 SkinMesh에 대한 Constant Buffer 생성
	mCB_BoneTransforms.resize(skinMeshCount);
	mCBMap_BoneTransforms.resize(skinMeshCount);

	size_t byteSize = (((sizeof(Vec4x4) * gkSkinBoneSize) + 255) & ~255); //256의 배수
	for (size_t i = 0; i < skinMeshCount; ++i) {
		D3DUtil::CreateBufferResource(nullptr, byteSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, mCB_BoneTransforms[i]);
		mCB_BoneTransforms[i]->Map(0, nullptr, (void**)&mCBMap_BoneTransforms[i]);
	}
}

AnimationController::~AnimationController()
{
	for (auto& CB_BoneTransform : mCB_BoneTransforms) {
		CB_BoneTransform->Unmap(0, nullptr);
	}
}

void AnimationController::SetTrackAnimationClip(int trackIndex, int clipIndex)
{
	mAnimationTracks[trackIndex].SetClip(clipIndex);
}

void AnimationController::SetTrackEnable(int trackIndex, bool isEnable)
{
	mAnimationTracks[trackIndex].SetEnable(isEnable);
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
	for (size_t i = 0; i < mSkinMeshes.size(); i++) {
		mSkinMeshes[i]->mCB_BoneTransforms = mCB_BoneTransforms[i];
		mSkinMeshes[i]->mCBMap_BoneTransforms = mCBMap_BoneTransforms[i];
	}
}

void AnimationController::Animate()
{
	for(auto& track : mAnimationTracks) {
		track.Animate(GetClip(track)->mLength);
	}

	auto& boneFrames = mSkinMeshes.front()->mBoneFrames;

	for (size_t i = 0; i < boneFrames.size(); i++) {
		Vec4x4 transform{ Matrix4x4::Zero() };
		for (const auto& track : mAnimationTracks) {
			if (!track.IsEnable()) {
				continue;
			}

			Vec4x4 trackTransform = GetClip(track)->GetSRT(i, track.GetPosition());
			Vec4x4 trackScale = Matrix4x4::Scale(trackTransform, track.GetWeight());
			transform = Matrix4x4::Add(transform, trackScale);
		}

		boneFrames[i]->SetLocalTransform(transform);
	}
}