#include "EnginePch.h"
#include "Animator.h"
#include "DXGIMgr.h"

#include "AnimatorController.h"
#include "AnimationClip.h"
#include "BattleScene.h"
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
	mAvatar = avatar;
	InitBoneFrames(skinMeshCount, avatar, animationInfo->IsManualBoneCalc);
}

void Animator::UpdateShaderVariables()
{
	for (size_t i = 0; i < mSkinMeshes.size(); ++i) {
		mSkinMeshes[i]->mBoneFrames = &mBoneFrames;
	}
}

void Animator::Animate()
{
	if (!mController) {
		return;
	}

	mController->Animate();

	UpdateTransform();
}

void Animator::UpdateTransform()
{
	if (mIsCloned) {
		return;
	}

	const auto& skinMesh = mSkinMeshes.front();
	for (const auto& [name, frame] : mBoneFrames) {
		Matrix transform = mController->GetTransform(name, skinMesh->GetHumanBone(name));
		frame->SetLocalTransform(transform, false);
	}
}

void Animator::CloneBoneFrames(Object* other)
{
	GameObject* gameObject = dynamic_cast<GameObject*>(other);
	if (!gameObject) {
		return;
	}

	mIsCloned = true;

	//for (auto& [name, frame] : mBoneFrames) {
	//	frame = gameObject->GetAnimator()->GetBoneFrame(name);
	//}
}

void Animator::InitController(rsptr<const AnimationLoadInfo> animationInfo)
{
	if (!animationInfo->AnimatorControllerFile.empty()) {
		mController = std::make_shared<AnimatorController>(*RESOURCE<AnimatorController>(animationInfo->AnimatorControllerFile));
	}
}

void Animator::InitBoneFrames(size_t skinMeshCount, GameObject* avatar, bool isManualBoneCalc)
{
	if (isManualBoneCalc) {
		auto& SetBoneFrame = [&](Transform* transform) {
			mBoneFrames[transform->GetObj<Object>()->GetName()] = transform;
			};

		Transform* root = avatar->FindFrame("root");
		root->DoAllChilds(SetBoneFrame);
	}
	else {
		for (const auto& skinMesh : mSkinMeshes) {
			for (const auto& boneName : skinMesh->mBoneNames) {
				mBoneFrames[boneName] = avatar->FindFrame(boneName);
			}
		}
	}
}