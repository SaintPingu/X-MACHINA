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
	bool mIsCheckTransition{};
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
	const AnimatorParameter* GetParamRef(const std::string& paramName) const { return &mParameters.at(paramName); }

	template<class T, typename std::enable_if<	std::is_same<T, bool>::value ||
												std::is_same<T, int>::value ||
												std::is_same<T, float>::value>::type* = nullptr>
	void SetValue(const std::string& paramName, T value) { SetValue(paramName, &value); }

public:
	void Animate();

	void SyncAnimation() const;

private:
	void InitLayers();
	void CheckTransition();

	void SetValue(const std::string& paramName, void* value);
};