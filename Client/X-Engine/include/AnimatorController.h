#pragma once

#pragma region Include
#include "Resources.h"
#pragma endregion

struct AnimationLoadInfo;
class AnimatorState;
class AnimatorStateMachine;
class AnimatorLayer;
class AnimatorMotion;
class HumanBone;

struct AnimatorParameter {
	enum class Type : BYTE {
		Float,
		Int,
		Bool,
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



class AnimatorController : public Resource {
	template <typename T>
	static constexpr bool is_valid_param_type = (std::is_same<T, bool>::value || std::is_same<T, int>::value || std::is_same<T, float>::value);

private:
	bool mIsPlayer{};
	bool mIsRemotePlayer{};
	Animations::ParamMap mParameters{};

	std::vector<sptr<AnimatorLayer>> mLayers;
	std::vector<AnimatorMotion*>	 mPrevMotions;
	std::unordered_map<int, std::string> mMotionMapInt{};
	std::unordered_map<std::string, int> mMotionMapString{};

	std::function<void()> mSendCallback{};

public:
	AnimatorController(const Animations::ParamMap& parameters, std::vector<sptr<AnimatorLayer>> layers);
	AnimatorController(const AnimatorController& other);
	virtual ~AnimatorController() = default;

	bool HasParam(const std::string paramName) const { return mParameters.contains(paramName); }

	Matrix GetTransform(const std::string& boneName, HumanBone boneType);
	const Animations::ParamMap& GetParams() const { return mParameters; }
	const AnimatorParameter* GetParam(const std::string& paramName) const { return &mParameters.at(paramName); }
	const AnimatorParameter* GetParamRef(const std::string& paramName) const { return &mParameters.at(paramName); }

	int GetMotionIndex(int layerIdx);

	template<class T, typename std::enable_if<is_valid_param_type<T>>::type* = nullptr>
	T GetParamValue(const std::string & paramName) const
	{
		if (std::is_same_v<T, bool>) {
			return mParameters.at(paramName).val.b;
		}
		if (std::is_same_v<T, int>) {
			return mParameters.at(paramName).val.i;
		}

		return mParameters.at(paramName).val.f;
	}

	template<class T, typename std::enable_if<is_valid_param_type<T>>::type* = nullptr>
	bool SetValueOnly(const std::string& paramName, T value)
	{
		if (!HasParam(paramName)) {
			return false;
		}

		AnimatorParameter::value val{};

		auto& param = mParameters[paramName];
		switch (param.type) {
		case AnimatorParameter::Type::Bool:
			val.b = static_cast<bool>(value);
			if (param.val.b == val.b) {
				return true;
			}
			break;
		case AnimatorParameter::Type::Int:
			val.i = static_cast<int>(value);
			if (param.val.i == val.i) {
				return true;
			}
			break;
		case AnimatorParameter::Type::Float:
			val.f = static_cast<float>(value);
			if (Math::IsEqual(param.val.f, val.f)) {
				return true;
			}
			break;
		default:
			assert(0);
			break;
		}

		param.val = val;
		return true;
	}

	// isChangeImmed == true : transition animation state without waiting & blending
	template<class T, typename std::enable_if<is_valid_param_type<T>>::type* = nullptr>
	void SetValue(const std::string& paramName, T value, bool isChangeImmed = false)
	{
		if (!SetValueOnly(paramName, value)) {
			return;
		}

		if(isChangeImmed) {
			CheckTransition(isChangeImmed);
		}
	}

	void SetAnimation(int upperIndex, int lowerIndex, float v, float h);
	void SetPlayer();
	void SetRemotePlayer() { mIsRemotePlayer = true; }

public:
	void Start();
	void Animate();
	void Release();

	void SyncAnimation() const;

	AnimatorMotion* FindMotionByName(const std::string& motionName, const std::string& layerName = "Base Layer") const;
	AnimatorMotion* GetCrntMotion(const std::string& layerName = "Base Layer") const;
	AnimatorMotion* GetLastMotion(const std::string& layerName = "Base Layer") const;

	bool IsEndTransition(const std::string& layerName) const;
	void SetAnimationSendCallback(const std::function<void()>& callback) { mSendCallback = callback; }

	void CheckTransition(bool isChangeImmed = false);

private:
	void InitLayers();

	AnimatorLayer* FindLayerByName(const std::string& layerName) const;
};	