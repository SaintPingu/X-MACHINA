#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Shader;
class Ability;
class Script_Player;
#pragma endregion


#pragma region EnumClass
enum class AbilityState : UINT8 {
	Ready = 0,
	Active,
	Cooldown
};
#pragma endregion


#pragma region Class
class Script_AbilityHolder : public Component {
	COMPONENT(Script_AbilityHolder, Component)

protected:
	sptr<Script_Player> mPlayer{};

	sptr<Ability>	mAbility{};
	float			mCooldownTime{};
	float			mActiveTime{};
	int				mAbilityCBIdx = -1;

	AbilityState	mState = AbilityState::Ready;

public:
	void SetAbility(sptr<Ability> ability);
	const AbilityState GetAbilityState() const { return mState; }
	const std::string& GetAbilityName() const;

public:
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void Toggle();
	void Terminate();

	void Enable();
	void Disable();
};

class Script_ToggleAbilityHolder : public Script_AbilityHolder {
	COMPONENT(Script_ToggleAbilityHolder, Script_AbilityHolder)

public:
	virtual void Update() override;
};

class Script_CooldownAbilityHolder : public Script_AbilityHolder {
	COMPONENT(Script_CooldownAbilityHolder, Script_AbilityHolder)

public:
	virtual void Toggle() override;
};
#pragma endregion
