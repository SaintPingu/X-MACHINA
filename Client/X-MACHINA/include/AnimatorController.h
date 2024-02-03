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

class AnimatorController {
private:
	std::unordered_map<std::string, AnimatorParameter> mParameters{};

	sptr<AnimatorState>	mCrntState{};
	sptr<AnimatorState>	mNextState{};

public:
	AnimatorController(const std::unordered_map<std::string, AnimatorParameter>& parameters, rsptr<AnimatorLayer> baseLayer);
	AnimatorController(const AnimatorController& other);
	virtual ~AnimatorController() = default;

public:
	void Animate();

	Vec4x4 GetTransform(int boneIndex);

	void SetBool(const std::string& name, bool value);

private:
	void ChangeToNextState();
};