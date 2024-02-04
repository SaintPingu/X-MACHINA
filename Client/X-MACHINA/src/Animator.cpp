#include "stdafx.h"
#include "Animator.h"
#include "DXGIMgr.h"

#include "AnimatorController.h"
#include "AnimationClip.h"
#include "Scene.h"
#include "Mesh.h"
#include "Timer.h"
#include "Transform.h"

#include "Object.h"


Animator::Animator(rsptr<const AnimationLoadInfo> animationInfo, GameObject* avatar)
{
	InitController(animationInfo);

	// Set skin meshes
	mSkinMeshes = animationInfo->SkinMeshes;
	const size_t skinMeshCount = mSkinMeshes.size();

	InitBoneFrames(skinMeshCount, avatar);

	InitBoneTransforms(skinMeshCount);
}

Animator::~Animator()
{
	for (auto& CB_BoneTransform : mCB_BoneTransforms) {
		CB_BoneTransform->Unmap(0, nullptr);
	}
}

void Animator::UpdateShaderVariables()
{
	for (size_t i = 0; i < mSkinMeshes.size(); ++i) {
		mSkinMeshes[i]->mBoneFrames = &mBoneFramesList[i];
		mSkinMeshes[i]->mCB_BoneTransforms = mCB_BoneTransforms[i];
		mSkinMeshes[i]->mCBMap_BoneTransforms = mCBMap_BoneTransforms[i];
	}
}

void Animator::Animate()
{
	mController->Animate();

	auto& boneFrames = mBoneFramesList.front();

	for (int j = 0; j < boneFrames.size(); ++j) {
		Vec4x4 transform{ Matrix4x4::Zero() };

		transform = mController->GetTransform(j);
		
		boneFrames[j]->SetLocalTransform(transform);
	}
}

void Animator::SetBool(const std::string& name, bool value)
{
	mController->SetBool(name, value);
}

void Animator::InitController(rsptr<const AnimationLoadInfo> animationInfo)
{
	mController = scene->GetAnimatorController(animationInfo->AnimatorControllerFile);
}

void Animator::InitBoneFrames(size_t skinMeshCount, GameObject* avatar)
{
	mBoneFramesList.resize(skinMeshCount);
	for (size_t i = 0; i < skinMeshCount; ++i) {
		const auto& boneNames = mSkinMeshes[i]->mBoneNames;
		auto& boneFrames = mBoneFramesList[i];

		boneFrames.resize(boneNames.size());
		for (size_t j = 0; j < boneNames.size(); ++j) {
			boneFrames[j] = avatar->FindFrame(boneNames[j])->GetObj<Transform>();
		}
	}
	std::sort(mBoneFramesList.begin(), mBoneFramesList.end(), [](const auto& first, const auto& second) {
		return first.size() > second.size();
		});
}

void Animator::InitBoneTransforms(size_t skinMeshCount)
{
	// 각 SkinMesh에 대한 Constant Buffer 생성
	mCB_BoneTransforms.resize(skinMeshCount);
	mCBMap_BoneTransforms.resize(skinMeshCount);

	size_t byteSize = D3DUtil::CalcConstantBuffSize(sizeof(Vec4x4) * gkSkinBoneSize);
	for (size_t i = 0; i < skinMeshCount; ++i) {
		D3DUtil::CreateBufferResource(nullptr, byteSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, mCB_BoneTransforms[i]);
		mCB_BoneTransforms[i]->Map(0, nullptr, (void**)&mCBMap_BoneTransforms[i]);
	}
}