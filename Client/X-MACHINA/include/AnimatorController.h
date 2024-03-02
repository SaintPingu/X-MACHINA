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

	std::vector<sptr<AnimatorLayer>> mLayers;

public:
	AnimatorController(const Animations::ParamMap& parameters, std::vector<sptr<AnimatorLayer>> layers);
	AnimatorController(const AnimatorController& other);
	virtual ~AnimatorController() = default;

	bool HasParam(const std::string paramName) const { return mParameters.contains(paramName); }

	Vec4x4 GetTransform(int boneIndex, HumanBone boneType);
	const Animations::ParamMap& GetParams() const { return mParameters; }
	const AnimatorParameter* GetParam(const std::string& paramName) const { return &mParameters.at(paramName); }
	float GetParamValue(const std::string& paramName) const { return mParameters.at(paramName).val.f; }

	void SetValue(const std::string& paramName, AnimatorParameter::value value);

public:
	void Animate();

	void SyncAnimation();

private:
	void InitLayers();
	void CheckTransition();
};