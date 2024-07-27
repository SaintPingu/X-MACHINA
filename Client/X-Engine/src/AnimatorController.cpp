#include "EnginePch.h"
#include "AnimatorController.h"

#include "Animator.h"
#include "AnimatorMotion.h"
#include "AnimatorLayer.h"
#include "AnimationClip.h"
#include "BattleScene.h"
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

void AnimatorController::SetAnimation(int upperIndex, int lowerIndex, float v, float h)
{
	auto& SetIndex = [&](int layerIndex, int index) {
		if (mMotionMapInt.count(index)) {
			std::string motionName = mMotionMapInt.at(index);
			mLayers[layerIndex]->SetAnimation(motionName);
		}
		else {
			std::cout << "[WARNING_SetAnimation()] Wrong animation index : " << index << std::endl;
		}
		};

	SetIndex(0, upperIndex);
	SetIndex(1, lowerIndex);
	std::cout << "\n";
}

void AnimatorController::SetPlayer()
{
	mIsPlayer = true;

	mPrevMotions.resize(mLayers.size());
	for(int i=0;i<mLayers.size();++i){
		mPrevMotions[i] = mLayers[i]->GetCrntMotion();
	}
}

void AnimatorController::Start()
{
	CheckTransition();
}

void AnimatorController::Animate()
{
	CheckTransition();

	for (auto& layer : mLayers) {
		layer->Animate();
	}
}

void AnimatorController::Release()
{
	for (auto& layer : mLayers) {
		layer->Release();
	}
	mLayers.clear();
}

Matrix AnimatorController::GetTransform(const std::string& boneName, HumanBone boneType)
{
	for (auto& layer : mLayers) {
		if (layer->CheckBoneMask(boneType)) {
			return layer->GetTransform(boneName, boneType);
		}
	}

	return Matrix::Identity;
}

int AnimatorController::GetMotionIndex(int layerIdx)
{
	const auto& motion = mPrevMotions[layerIdx];
	if (mMotionMapString.count(motion->GetName())) {
		if (mLayers[layerIdx]->FindMotionByName(motion->GetName())) {
			return mMotionMapString.at(motion->GetName());
		}
	}

	std::cout << "[WARNING_GetMotionIndex()] Wrong motion : " << motion->GetName() << std::endl;
	return -1;
}

void AnimatorController::SyncAnimation() const
{
	if (mLayers.size() >= 2) {
		auto& srcLayer = mLayers.back();
		const AnimatorMotion* srcState = srcLayer->GetSyncState();

		for (size_t i = 0; i < mLayers.size(); ++i) {
			if (mLayers[i] == srcLayer) {
				continue;
			}
			mLayers[i]->SyncAnimation(srcState);
		}
	}
}

AnimatorMotion* AnimatorController::FindMotionByName(const std::string& motionName, const std::string& layerName) const
{
	return FindLayerByName(layerName)->FindMotionByName(motionName);
}

AnimatorMotion* AnimatorController::GetCrntMotion(const std::string& layerName) const
{
	return FindLayerByName(layerName)->GetCrntMotion();
}

AnimatorMotion* AnimatorController::GetLastMotion(const std::string& layerName) const
{
	return FindLayerByName(layerName)->GetLastMotion();
}

bool AnimatorController::IsEndTransition(const std::string& layerName) const
{
	return FindLayerByName(layerName)->IsEndTransition();
}

void AnimatorController::CheckTransition(bool isChangeImmed)
{
	if (mIsRemotePlayer) {
		return;
	}

	bool isSend = false;
	int i{};
	for (auto& layer : mLayers) {
		auto nextMotion = layer->CheckTransition(this, isChangeImmed);
		if (mSendCallback && nextMotion) {
			if (mPrevMotions[i] != nextMotion) {
				mPrevMotions[i] = nextMotion;
				isSend = true;
			}
		}
		++i;
	}

	if (mIsPlayer && isSend) {
		mSendCallback();
	}
}

void AnimatorController::InitLayers()
{
	int index = 0;
	for (auto& layer : mLayers) {
		layer->Init(this);
		if (layer->GetName().contains("Legs")) {
			layer->SetSyncStateMachine(true);
		}
		layer->AddStates(index, mMotionMapInt, mMotionMapString);
	}
	CheckTransition(true);
}



AnimatorLayer* AnimatorController::FindLayerByName(const std::string& layerName) const
{
	for (auto& layer : mLayers) {
		if (layer->GetName() == layerName) {
			return layer.get();
		}
	}

	throw std::runtime_error("there's no layer name in controller");
}