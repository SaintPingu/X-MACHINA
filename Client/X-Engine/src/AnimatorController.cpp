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
	Resource(ResourceType::AnimatorController),
	mParameters(other.mParameters)
{
	// layers 복사 하기
	mLayers.reserve(other.mLayers.size());
	for (auto& layer : other.mLayers) {
		mLayers.push_back(std::make_shared<AnimatorLayer>(*layer));
	}

	InitLayers();
}

void AnimatorController::Animate()
{
	if (mIsCheckTransition) {
		mIsCheckTransition = false;
		CheckTransition();
	}

	for (auto& layer : mLayers) {
		layer->Animate();
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

bool AnimatorController::IsEndTransition(const std::string& layerName) const
{
	return FindLayerByName(layerName)->IsEndTransition();
}

void AnimatorController::CheckTransition()
{
	for (auto& layer : mLayers) {
		layer->CheckTransition(this);
	}
}

void AnimatorController::InitLayers()
{
	for (auto& layer : mLayers) {
		layer->Init(this);
		if (layer->GetName().contains("Legs")) {
			layer->SetSyncStateMachine(true);
		}
	}
	CheckTransition();
}

void AnimatorController::SetValue(const std::string& paramName, void* value)
{
	if (!HasParam(paramName)) {
		return;
	}

	AnimatorParameter::value val{};

	auto& param = mParameters[paramName];
	switch (param.type) {
	case AnimatorParameter::Type::Bool:
		val.b = *(bool*)value;
		if (param.val.b == val.b) {
			return;
		}
		break;
	case AnimatorParameter::Type::Int:
		val.i = *(int*)value;
		if (param.val.i == val.i) {
			return;
		}
		break;
	case AnimatorParameter::Type::Float:
		val.f = *(float*)value;
		if (Math::IsEqual(param.val.f, val.f)) {
			return;
		}
		break;
	}

	mParameters[paramName].val = val;
	mIsCheckTransition = true;
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
