#include "EnginePch.h"
#include "Animator.h"
#include "DXGIMgr.h"

#include "AnimatorController.h"
#include "AnimationClip.h"
#include "Scene.h"
#include "Mesh.h"
#include "Timer.h"
#include "Component/Transform.h"
#include "ResourceMgr.h"

#include "Object.h"


Animator::Animator(rsptr<const AnimationLoadInfo> animationInfo, GameObject* avatar)
{
	InitController(animationInfo);

	// Set skin meshes
	mSkinMeshes = animationInfo->SkinMeshes;
	const size_t skinMeshCount = mSkinMeshes.size();

	InitBoneFrames(skinMeshCount, avatar);
}

Animator::~Animator()
{

}

void Animator::UpdateShaderVariables()
{
	for (size_t i = 0; i < mSkinMeshes.size(); ++i) {
		mSkinMeshes[i]->mBoneFrames = &mBoneFramesList[i];
	}
}

void Animator::Animate()
{
	if (!mController) {
		return;
	}

	mController->Animate();

	auto& boneFrames = mBoneFramesList.front();
	auto& skinMesh = mSkinMeshes.front();

	for (int j = 0; j < boneFrames.size(); ++j) {
		Matrix transform = mController->GetTransform(j, skinMesh->GetHumanBone(j));
		
		boneFrames[j]->SetLocalTransform(transform);
	}
}

void Animator::InitController(rsptr<const AnimationLoadInfo> animationInfo)
{
	if (!animationInfo->AnimatorControllerFile.empty()) {
		mController = std::make_shared<AnimatorController>(*RESOURCE<AnimatorController>(animationInfo->AnimatorControllerFile));
	}
}

void Animator::InitBoneFrames(size_t skinMeshCount, GameObject* avatar)
{
	// Bone이 가장 많은 것이 맨 앞으로
	std::sort(mSkinMeshes.begin(), mSkinMeshes.end(), [](const auto& first, const auto& second) {
		return first->mBoneNames.size() > second->mBoneNames.size();
		});

	mBoneFramesList.resize(skinMeshCount);
	for (size_t i = 0; i < skinMeshCount; ++i) {
		const auto& boneNames = mSkinMeshes[i]->mBoneNames;
		auto& boneFrames = mBoneFramesList[i];

		boneFrames.resize(boneNames.size());
		for (size_t j = 0; j < boneNames.size(); ++j) {
			boneFrames[j] = avatar->FindFrame(boneNames[j]);
		}
	}
}