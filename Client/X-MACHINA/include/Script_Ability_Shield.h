#pragma once


#pragma region Include
#include "Script_Ability.h"
#include "PheroAbilityInterface.h"
#pragma endregion

class Script_PheroPlayer;

#pragma region Class
class Script_Ability_Shield : public Script_RenderedAbility, public PheroAbilityInterface {
	COMPONENT(Script_Ability_Shield, Script_RenderedAbility)

private:
	sptr<Script_PheroPlayer> mPlayer{};
	float mShieldAmount{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;

public:
	virtual void On() override;
	virtual void Off() override;

protected:
	virtual bool ReducePheroAmount(bool checkOnly = false) override;
};
#pragma endregion

