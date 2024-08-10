#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class GameObject;
class Mesh;
class Shader;
#pragma endregion



class Script_Ability abstract : public Component {
	COMPONENT_ABSTRACT(Script_Ability, Component)

protected:
	enum class Type{ Default, Toggle, Cooldown };

public:
	enum class State{ Ready, Active, Cooldown };

private:
	Type mType{};
	void (Script_Ability::* mUpdateFunc)() { &Script_Ability::UpdateDefault };

protected:
	float		mMaxCooldownTime{};
	float		mCurCooldownTime{};
	float		mMaxActiveTime{};
	float		mCurActiveTime{};
	std::string mAbilityName{};

	State	mState{};
public:
	virtual void Update() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;

public:
	const std::string& GetName() const { return mAbilityName; }
	State GetState() const { return mState; }
	bool IsActiveState() const { return mState == State::Active; }

public:
	virtual bool ProcessInput() { return true; }
	virtual bool ProcessMouseMsg(UINT messageID, WPARAM wParam, LPARAM lParam) { return true; }
	virtual bool ProcessKeyboardMsg(UINT messageID, WPARAM wParam, LPARAM lParam) { return true; }

	void Toggle();

protected:
	virtual void Init(std::string abilityName, float cooldownTime = 0.f, float activeTime = 0.f);

	virtual void On();
	virtual void Off();
	virtual void Ready();

	void SetType(Type type);

private:
	void UpdateDefault();
	void UpdateToggle();
};


class Script_RenderedAbility : public Script_Ability {
	COMPONENT(Script_RenderedAbility, Script_Ability)

protected:
	sptr<GameObject>	mRenderedObject;
	sptr<Mesh>			mRenderedMesh;

	sptr<Shader>		mShader{};
	int					mLayer{};

	int					mAbilityCBIdx = -1;
	AbilityConstants	mAbilityCB{};

public:
	virtual void Update() override;

public:
	int GetAbilityCBIdx() const { return mAbilityCBIdx; }

public:
	virtual void On() override;
	virtual void Off() override;
	virtual void Render();

private:
	void UpdateAbilityCB(float activeTime = 0.f);

};
