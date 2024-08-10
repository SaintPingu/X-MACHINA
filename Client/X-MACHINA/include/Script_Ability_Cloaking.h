#pragma once


#pragma region Include
#include "Script_Ability.h"
#include "PheroAbilityInterface.h"
#pragma endregion



class Script_Ability_Cloaking : public Script_RenderedAbility, public PheroAbilityInterface {
	COMPONENT(Script_Ability_Cloaking, Script_RenderedAbility)

public:
	sptr<class Script_PheroPlayer> mPlayer{};
	class ParticleSystem* mBuffPS{};
	sptr<class Script_AfterImageObject> mAfterImage{};
	ObjectTag mPrevInvokerTag = ObjectTag::Player;

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

class Script_Remote_Ability_Cloaking : public Script_Ability_Cloaking {
	COMPONENT(Script_Remote_Ability_Cloaking, Script_Ability_Cloaking)

public:
	virtual void Update() override;
	virtual void On() override;
	virtual void Off() override;
};

