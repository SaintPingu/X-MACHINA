#include "stdafx.h"
#include "AnimatorMotion.h"

#include "AnimationClip.h"
#include "AnimatorController.h"

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
	mWeight = other.mWeight;
	mPosition = other.mPosition;
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

	bool onlyOne = false;
	Vec2 position{ -x->val.f, -y->val.f };
	for (size_t i = 0; i < mMotions.size(); ++i) {
		float distance = Vector2::LengthExact(position, mMotions[i]->GetPosition());
		weights[i] += max(0, 1 - distance);
		totalWight += weights[i];

		if (weights[i] > 0.9f) {
			onlyOne = true;
		}
	}

	if (onlyOne) {
		for (size_t i = 0; i < mMotions.size(); ++i) {
			if (weights[i] > 0.9f) {
				mMotions[i]->SetWeight(1.f);
			}
			else {
				mMotions[i]->SetWeight(0.f);
			}
		}
		return;
	}

	for (size_t i = 0; i < mMotions.size(); ++i) {
		float weight = weights[i] / totalWight;
		if (weight < 0.00001f) {
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