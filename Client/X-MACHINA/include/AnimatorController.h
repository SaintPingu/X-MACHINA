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

	Type type{};
	union value {
		bool b;
		int i;
		float f;
	} val;
};

class AnimatorController {
private:
	std::unordered_map<std::string, AnimatorParameter> mParameters{};

	std::unordered_map<std::string, sptr<AnimatorState>> mStates{};
	sptr<AnimatorState>	mCrntState{};
	sptr<AnimatorState>	mNextState{};

public:
	AnimatorController(const std::unordered_map<std::string, AnimatorParameter>& parameters, const std::unordered_map<std::string, sptr<AnimatorState>>& states);
	AnimatorController(const AnimatorController& other);
	virtual ~AnimatorController() = default;

public:
	void Animate();

	Vec4x4 GetTransform(int boneIndex);

	void SetBool(const std::string& name, bool value);

private:
	void ChangeToNextState();
};