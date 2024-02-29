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

}

AnimatorController::AnimatorController(const AnimatorController& other)
	:
	mParameters(other.mParameters)
{
	mLayers.reserve(other.mLayers.size());
	for (auto& layer : other.mLayers) {
		mLayers.push_back(std::make_shared<AnimatorLayer>(*layer));
	}
	// layers 복사 하기
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

void AnimatorController::CheckTransition()
{
	for (auto& layer : mLayers) {
		layer->CheckTransition(this);
	}
}