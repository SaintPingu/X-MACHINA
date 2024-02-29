#include "stdafx.h"
#include "AnimatorController.h"

#include "Animator.h"
#include "AnimatorState.h"
#include "AnimatorLayer.h"
#include "AnimationClip.h"
#include "Scene.h"
#include "Timer.h"

AnimatorController::AnimatorController(const Animations::ParamMap& parameters, std::vector<sptr<AnimatorLayer>> layers)
	:
	mParameters(parameters),
	mLayers(layers)
{
	for (auto& layer : mLayers) {
		layer->SetController(this);
	}
}

AnimatorController::AnimatorController(const AnimatorController& other)
	:
	mParameters(other.mParameters)
{
	// layers 복사 하기
	mLayers.reserve(other.mLayers.size());
	for (auto& layer : other.mLayers) {
		mLayers.push_back(std::make_shared<AnimatorLayer>(*layer));
	}

	for (auto& layer : mLayers) {
		layer->SetController(this);
	}
}

void AnimatorController::Animate()
{
	for (auto& layer : mLayers) {
		layer->Animate();
	}
}

Vec4x4 AnimatorController::GetTransform(int boneIndex, HumanBone boneType)
{
	for (auto& layer : mLayers) {
		if (layer->CheckBoneMask(boneType)) {
			return layer->GetTransform(boneIndex, boneType);
		}
	}

	return Matrix4x4::Identity();

	//std::vector<Vec4x4> transforms;
	//transforms.reserve(mLayers.size());

	//for (auto& layer : mLayers) {
	//	if (layer->CheckBoneMask(boneType)) {
	//		transforms.push_back(layer->GetTransform(boneIndex, boneType));
	//	}
	//}

	//if (transforms.empty()) {
	//	return Matrix4x4::Identity();
	//}

	//if (transforms.size() <= 1) {
	//	return transforms.front();
	//}

	//Vec4x4 result = Matrix4x4::Scale(transforms[0], 0.8f);
	//result = Matrix4x4::Add(result, Matrix4x4::Scale(transforms[1], 0.2f));
	//return result;
}

void AnimatorController::SetValue(const std::string& paramName, AnimatorParameter::value value)
{
	if (!HasParam(paramName)) {
		return;
	}

	auto& param = mParameters[paramName];
	switch (param.type) {
	case AnimatorParameter::Type::Bool:
	case AnimatorParameter::Type::Trigger:
		if (param.val.b == value.b) {
			return;
		}
		break;
	case AnimatorParameter::Type::Int:
		if (param.val.i == value.i) {
			return;
		}
		break;
	case AnimatorParameter::Type::Float:
		if (Math::IsEqual(param.val.f,value.f)) {
			return;
		}
		break;
	}

	mParameters[paramName].val = value;

	CheckTransition();
}

void AnimatorController::SyncAnimation()
{
	if (mLayers.size() >= 2) {
		auto& srcLayer = mLayers.back();
		rsptr<const AnimatorState> srcState = srcLayer->GetSyncState();

		for (size_t i = 0; i < mLayers.size(); ++i) {
			if (mLayers[i] == srcLayer) {
				continue;
			}
			mLayers[i]->SyncAnimation(srcState);
		}
	}
}

void AnimatorController::CheckTransition()
{
	for (auto& layer : mLayers) {
		layer->CheckTransition(this);
	}
}