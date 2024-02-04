#pragma once

struct AnimationLoadInfo;
class AnimatorState;
class AnimatorLayer;

struct AnimatorParameter {
	enum class Type {
		Float,
		Int,
		Bool,
		Trigger
	};

	Type type{};
	union value {
		bool b;
		int i;
		float f;
	} val{};
};

namespace Animations {
	using ParamMap = std::unordered_map<std::string, AnimatorParameter>;
}

class AnimatorController {
private:
	Animations::ParamMap mParameters{};

	const sptr<AnimatorLayer> mBaseLayer{};
	sptr<AnimatorLayer> mCrntLayer{};

	sptr<AnimatorState>	mCrntState{};
	sptr<AnimatorState>	mNextState{};

public:
	AnimatorController(const Animations::ParamMap& parameters, rsptr<AnimatorLayer> baseLayer);
	AnimatorController(const AnimatorController& other);
	virtual ~AnimatorController() = default;

public:
	void Animate();

	Vec4x4 GetTransform(int boneIndex);

	void SetBool(const std::string& name, bool value);

private:
	void ChangeToNextState();
};