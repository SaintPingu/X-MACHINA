#pragma once


#pragma region Include
#include "AbilityMgr.h"
#include "PheroAbilityInterface.h"
#pragma endregion


#pragma region Class
class CloakingAbility : public Ability, public PheroAbilityInterface {
	using base = Ability;

public:
	CloakingAbility();
	class ParticleSystem* mBuffSparkPS;
	class ParticleSystem* mBuffDotPS;

	sptr<class GameObject> mAfterImageObject;

public:
	virtual void Update(float activeTime) override;
	virtual void Activate() override;
	virtual void DeActivate() override;

protected:
	virtual bool ReducePheroAmount(bool checkOnly = false) override;
};
#pragma endregion
