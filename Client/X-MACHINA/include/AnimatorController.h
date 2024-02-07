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

	bool HasParam(const std::string paramName) const { return mParameters.contains(paramName); }

	Vec4x4 GetTransform(int boneIndex);
	const Animations::ParamMap& GetParams() const { return mParameters; }
	const AnimatorParameter* GetParam(const std::string& paramName) const { return &mParameters.at(paramName); }
	float GetParamValue(const std::string& paramName) const { return mParameters.at(paramName).val.f; }

	void SetValue(const std::string& paramName, AnimatorParameter::value value);

public:
	void Animate();

private:

	void CheckTransition();
	void ChangeToNextState();
};