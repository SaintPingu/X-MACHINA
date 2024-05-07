#pragma once


#pragma region Include
#include "Component/Component.h"
#pragma endregion


#pragma region ClassForwardDecl
class Shader;
class Ability;
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

private:
	sptr<Ability> mAbility{};
	float mCooldownTime{};
	float mActiveTime{};

	AbilityState mState = AbilityState::Ready;
	int mKey = -1;

public:
	void SetAbility(int key, sptr<Ability> ability) { mKey = key, mAbility = ability; }

public:
	virtual void Start() override;
	virtual void Update() override;
};
#pragma endregion
