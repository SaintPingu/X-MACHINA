#include "stdafx.h"
#include "AnimatorState.h"

#include "AnimatorController.h"
#include "AnimatorStateMachine.h"
#include "AnimationClip.h"
#include "Timer.h"

AnimatorMotion::AnimatorMotion(rsptr<const AnimatorStateMachine> stateMachine, const std::vector<sptr<const AnimatorTransition>>& transitions, const std::string& name, float maxLength)
	:
	mName(name),
	mMaxLength(maxLength),
	mStateMachine(stateMachine.get()),
	mTransitions(transitions)
{

}

AnimatorMotion::AnimatorMotion(const AnimatorMotion& other)
{
	mName         = other.mName;
	mSpeed        = other.mSpeed;
	mCrntLength   = other.mCrntLength;
	mMaxLength    = other.mMaxLength;
	mWeight       = other.mWeight;
	mStateMachine = other.mStateMachine;
	mTransitions  = other.mTransitions;
}

void AnimatorMotion::ResetLength()
{
	if (IsReverse()) {
		mCrntLength = mMaxLength;
	}
	else {
		mCrntLength = 0.f;
	}
}

void AnimatorMotion::SetLength(float length)
{
	mCrntLength = length;
	if (IsEndAnimation()) {
		ResetLength();
	}
}

void AnimatorMotion::Reset()
{
	mCrntLength = 0.f;
	mWeight     = 1.f;
	mSpeed      = 1.f;
	if (IsReverse()) {
		Reverse();
	}
}


bool AnimatorMotion::IsEndAnimation() const
{
	return (mCrntLength > mMaxLength) || (mCrntLength <= 0);
}

bool AnimatorMotion::IsSameStateMachine(rsptr<const AnimatorMotion> other) const
{
	return mStateMachine == other->mStateMachine;
}

bool AnimatorMotion::Animate()
{
	constexpr float corrSpeed = 0.5f;	// 스피드 보정값 (60fps)

	mCrntLength += (mSpeed * corrSpeed * mIsReverse) * DeltaTime();
	if (IsEndAnimation()) {
		ResetLength();
		return true;
	}

	return false;
}



AnimatorTrack::AnimatorTrack(rsptr<const AnimationClip> clip)
	:
	mClip(clip)
{

}

AnimatorTrack::AnimatorTrack(const AnimatorTrack& other)
{
	mClip = other.mClip;
}

Vec4x4 AnimatorTrack::GetSRT(int boneIndex, float length) const
{
	return mClip->GetSRT(boneIndex, length);
}



AnimatorState::AnimatorState(rsptr<const AnimatorStateMachine> stateMachine, const std::vector<sptr<const AnimatorTransition>>& transitions, rsptr<const AnimationClip> clip)
	:
	AnimatorMotion(stateMachine, transitions, clip->mName, clip->mLength),
	AnimatorTrack(clip)
{

}

AnimatorState::AnimatorState(const AnimatorState& other)
	:
	AnimatorMotion(other),
	AnimatorTrack(other)
{

}

Vec4x4 AnimatorState::GetSRT(int boneIndex) const
{
	return AnimatorTrack::GetSRT(boneIndex, GetLength());
}



ChildMotion::ChildMotion(rsptr<const AnimationClip> clip, const Vec2& position)
	:
	AnimatorTrack(clip),
	mPosition(position)
{
	
}

ChildMotion::ChildMotion(const ChildMotion& other)
	:
	AnimatorTrack(other)
{
	mCrntLength = other.mCrntLength;
	mWeight     = other.mWeight;
	mPosition   = other.mPosition;
}

Vec4x4 ChildMotion::GetSRT(int boneIndex) const
{
	return AnimatorTrack::GetSRT(boneIndex, GetLength());
}



BlendTree::BlendTree(rsptr<const AnimatorStateMachine> stateMachine, const std::vector<sptr<const AnimatorTransition>>& transitions, const std::string& name, std::vector<sptr<ChildMotion>> motions)
	:
	AnimatorMotion(stateMachine, transitions, name, motions.front()->GetClip()->mLength),
	mMotions(motions)
{
	
}


BlendTree::BlendTree(const BlendTree& other)
	:
	AnimatorMotion(other)
{
	mMotions.reserve(other.mMotions.size());
	for (const auto& motion : other.mMotions) {
		mMotions.push_back(std::make_shared<ChildMotion>(*motion));
	}
}


void BlendTree::CalculateWeights() const
{
	std::vector<float> weights(mMotions.size());
	float totalWight{};

	Vec2 position{-x->val.f, -y->val.f};
	for (size_t i = 0; i < mMotions.size(); ++i) {
		float distance = Vector2::Length(position, mMotions[i]->GetPosition());
		weights[i] += max(0, 1 - distance);
		totalWight += weights[i];
	}

	for (size_t i = 0; i < mMotions.size(); ++i) {
		float weight = weights[i] / totalWight;
		if (weight < 0.01f) {
			weight = 0.f;
		}

		mMotions[i]->SetWeight(weight);
	}
}

Vec4x4 BlendTree::GetSRT(int boneIndex) const
{
	Vec4x4 transform{};
	for (auto& motion : mMotions) {
		float weight = motion->GetWeight();
		if (weight > 0.f) {
			transform = Matrix4x4::Add(transform, Matrix4x4::Scale(motion->GetSRT(boneIndex), weight));
		}
	}

	return transform;
}

void BlendTree::Init(const AnimatorController* controller)
{
	x = controller->GetParamRef("Horizontal");
	y = controller->GetParamRef("Vertical");
}

bool BlendTree::Animate()
{
	bool result = base::Animate();

	for (auto& motion : mMotions) {
		motion->SetLength(GetLength());
	}

	CalculateWeights();

	return result;
}