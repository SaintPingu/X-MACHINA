#pragma once

struct AnimationLoadInfo;
class AnimatorState;

struct AnimatorParameter {
	enum class Type {
		Float,
		Int,
		Bool,
		Trigger
	};

	std::string name{};
	Type type{};
	union value {
		bool b;
		int i;
		float f;
	} val;
};

class AnimatorController {
private:
	std::vector<AnimatorParameter> mParameters{};

	std::vector<sptr<AnimatorState>> mStates{};
	sptr<AnimatorState>				 mCrntState{};

public:
	AnimatorController(const std::vector<AnimatorParameter>& parameters, const std::vector<sptr<AnimatorState>>& states);
	AnimatorController(const AnimatorController& other);
	virtual ~AnimatorController() = default;

public:
	void Animate();

	Vec4x4 GetTransform(int boneIndex);
};