#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion

class Script_Weapon_Deus_Rifle;

#pragma region Class
class Script_Deus_Phase_1 : public Script_Enemy {
	COMPONENT(Script_Deus_Phase_1, Script_Enemy)

private:
	sptr<Script_Weapon_Deus_Rifle> mWeapon{};

protected:
	struct DeusPhase1AttackType : AttackType {
		static constexpr int MeleeAttack = 1;
		static constexpr int RangeAttack = 2;
		static constexpr int ExplodeAttack = 3;
		static constexpr int _count = 4;
	};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void StartAttack() override;
		
protected:
	void MeleeAttackCallback();
	void RangeAttackCallback();
	void ExplodeAttackCallback();

private:
	void FireMissille();
};
#pragma endregion
