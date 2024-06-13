#pragma once


#pragma region Include
#include "AbilityMgr.h"
#include "PheroAbilityInterface.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_LiveObject;
class GameObject;
#pragma endregion


#pragma region Class
class ShieldAbility : public RenderedAbility, public PheroAbilityInterface {
	using base = RenderedAbility;
private:
	float mShieldAmount{};

public:
	ShieldAbility(float sheild);

public:
	virtual void Update(float activeTime) override;
	virtual void Activate() override;
	virtual void DeActivate() override;

protected:
	virtual bool ReducePheroAmount() override;
};
#pragma endregion
