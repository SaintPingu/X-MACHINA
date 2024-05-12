#include "EnginePch.h"
#include "AnimatorController.h"

#include "Animator.h"
#include "AnimatorMotion.h"
#include "AnimatorLayer.h"
#include "AnimationClip.h"
#include "Scene.h"
#include "Timer.h"

AnimatorController::AnimatorController(const Animations::ParamMap& parameters, std::vector<sptr<AnimatorLayer>> layers)
	:
	Resource(ResourceType::AnimatorController),
	mParameters(parameters),
	mLayers(layers)
{
	InitLayers();
}

AnimatorController::AnimatorController(const AnimatorController& other)
	:
	Resource(other),
	mParameters(other.mParameters)
{
	// layers 복사 하기
	mLayers.reserve(other.mLayers.size());
	for (auto& layer : other.mLayers) {
		mLayers.push_back(std::make_shared<AnimatorLayer>(*layer));
	}

	InitLayers();
}

void AnimatorController::SetAnimation(int motionIndex)
{
	if (mMotionMapInt.count(motionIndex)) {
		std::string motionName = mMotionMapInt.at(motionIndex);
		for (auto& layer : mLayers) {
			if (layer->SetAnimation(motionName)) {
				break;
			}
		}
	}
}

void AnimatorController::Start()
{
	CheckTransition();
}

void AnimatorController::Animate()
{
	if (mIsCheckTransition) {
		UpdateTransition();
	}

	for (auto& layer : mLayers) {
		layer->Animate();
	}

	if (mIsCheckTransition) {
		UpdateTransition();
	}
}

Matrix AnimatorController::GetTransform(int boneIndex, HumanBone boneType)
{
	for (auto& layer : mLayers) {
		if (layer->CheckBoneMask(boneType)) {
			return layer->GetTransform(boneIndex, boneType);
		}
	}

	return Matrix::Identity;
}

void AnimatorController::SyncAnimation() const
{
	if (mLayers.size() >= 2) {
		auto& srcLayer = mLayers.back();
		rsptr<const AnimatorMotion> srcState = srcLayer->GetSyncState();

		for (size_t i = 0; i < mLayers.size(); ++i) {
			if (mLayers[i] == srcLayer) {
				continue;
			}
			mLayers[i]->SyncAnimation(srcState);
		}
	}
}

sptr<AnimatorMotion> AnimatorController::FindMotionByName(const std::string& motionName, const std::string& layerName) const
{
	return FindLayerByName(layerName)->FindMotionByName(motionName);
}

sptr<AnimatorMotion> AnimatorController::GetCrntMotion(const std::string& layerName) const
{
	return FindLayerByName(layerName)->GetCrntMotion();
}

sptr<AnimatorMotion> AnimatorController::GetLastMotion(const std::string& layerName) const
{
	return FindLayerByName(layerName)->GetLastMotion();
}

bool AnimatorController::IsEndTransition(const std::string& layerName) const
{
	return FindLayerByName(layerName)->IsEndTransition();
}

void AnimatorController::CheckTransition(bool isChangeImmed) const
{
	if (!mIsPlayer) {
		return;
	}

	rsptr<AnimatorMotion> nextMotion = mLayers[0]->CheckTransition(this, isChangeImmed);
	if (nextMotion && mSendCallback) {
		if (mMotionMapString.count(nextMotion->GetName())) {
			int index = mMotionMapString.at(nextMotion->GetName());
			mSendCallback(index);
		}
	}
}

void AnimatorController::UpdateTransition()
{
	CheckTransition();
	mIsCheckTransition = false;
}

void AnimatorController::InitLayers()
{
	int index = 0;
	for (auto& layer : mLayers) {
		layer->Init(this);
		if (layer->GetName().contains("Legs")) {
			layer->SetSyncStateMachine(true);
		}
		else {
			layer->AddStates(index, mMotionMapInt, mMotionMapString);
		}
	}
	CheckTransition(true);
}



sptr<AnimatorLayer> AnimatorController::FindLayerByName(const std::string& layerName) const
{
	for (auto& layer : mLayers) {
		if (layer->GetName() == layerName) {
			return layer;
		}
	}

	throw std::runtime_error("there's no layer name in controller");
}