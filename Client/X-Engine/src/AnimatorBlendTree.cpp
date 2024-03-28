#include "EnginePch.h"
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

Matrix ChildMotion::GetSRT(int boneIndex) const
{
	return AnimatorTrack::GetSRT(boneIndex, GetLength());
}



BlendTree::BlendTree(const AnimatorMotionInfo& info, std::vector<sptr<ChildMotion>> motions)
	:
	AnimatorMotion(AnimatorMotionInfo{ info.Name, motions.front()->GetClip()->mLength, info.Speed, info.StateMachine, info.Transitions }),
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
		float distance = (position + mMotions[i]->GetPosition()).Length();
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

Matrix BlendTree::GetSRT(int boneIndex) const
{
	Matrix transform = Matrix4x4::Zero();
	for (auto& motion : mMotions) {
		float weight = motion->GetWeight();
		if (weight > 0.f) {
			transform += (motion->GetSRT(boneIndex) * weight);
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