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

	InitBoneFrames(skinMeshCount, avatar, animationInfo->IsManualBoneCalc);

	if (animationInfo->IsManualBoneCalc) {
		mUpdateTransformFunc = std::bind(&Animator::UpdateTransformManual, this);
	}
	else {
		mUpdateTransformFunc = std::bind(&Animator::UpdateTransform, this);
	}
}

Animator::~Animator()
{

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

	mUpdateTransformFunc();
}

void Animator::UpdateTransform()
{
	if (mIsCloned) {
		return;
	}

	auto& skinMesh = mSkinMeshes.front();

	for (int i = 0; i < mBoneFrames.size(); ++i) {
		Matrix transform = mController->GetTransform(i, skinMesh->GetHumanBone(i));

		mBoneFrames[i]->SetLocalTransform(transform, false);
	}
}

void Animator::UpdateTransformManual()
{
	auto& skinMesh = mSkinMeshes.front();

	for (int i = 0; i < mFrames.size(); ++i) {
		Matrix transform = mController->GetTransform(i, HumanBone::None);

		mFrames[i]->SetLocalTransform(transform, false);
	}
}

void Animator::CloneBoneFrames(Object* other)
{
	GameObject* gameObject = dynamic_cast<GameObject*>(other);
	if (!gameObject) {
		return;
	}

	mIsCloned = true;

	for (int i = 0; i < mBoneFrames.size(); ++i) {
		*mBoneFrames[i] = gameObject->GetAnimator()->GetBoneFrame(i);
	}
}

void Animator::InitController(rsptr<const AnimationLoadInfo> animationInfo)
{
	if (!animationInfo->AnimatorControllerFile.empty()) {
		mController = std::make_shared<AnimatorController>(*RESOURCE<AnimatorController>(animationInfo->AnimatorControllerFile));
	}
}

void Animator::InitBoneFrames(size_t skinMeshCount, GameObject* avatar, bool isManualBoneCalc)
{
	// Bone이 가장 많은 것이 맨 앞으로
	std::sort(mSkinMeshes.begin(), mSkinMeshes.end(), [](const auto& first, const auto& second) {
		return first->mBoneNames.size() > second->mBoneNames.size();
		});

	const auto& boneNames = mSkinMeshes.front()->mBoneNames;
	mBoneFrames.resize(boneNames.size());

	for (size_t i = 0; i < boneNames.size(); ++i) {
		mBoneFrames[i] = avatar->FindFrame(boneNames[i]);
	}

	if (isManualBoneCalc) {
		auto& SetBoneFrame = [&](Transform* transform) {
			mFrames.push_back(transform);
			};

		rsptr<Transform> root = avatar->mChild;
		assert(root->GetObj<Object>()->GetName() == "root");
		root->DoAllChilds(SetBoneFrame);
	}
}