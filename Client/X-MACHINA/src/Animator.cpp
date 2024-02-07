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
	mController->Animate();

	auto& boneFrames = mBoneFramesList.front();

	for (int j = 0; j < boneFrames.size(); ++j) {
		Vec4x4 transform{ Matrix4x4::Zero() };

		transform = mController->GetTransform(j);
		
		boneFrames[j]->SetLocalTransform(transform);
	}
}

void Animator::SetValue(const std::string& paramName, bool value)
{
	AnimatorParameter::value val;
	val.b = value;
	mController->SetValue(paramName, val);
}
void Animator::SetValue(const std::string& paramName, int value)
{
	AnimatorParameter::value val;
	val.i = value;
	mController->SetValue(paramName, val);
}
void Animator::SetValue(const std::string& paramName, float value)
{
	AnimatorParameter::value val;
	val.f = value;
	mController->SetValue(paramName, val);
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