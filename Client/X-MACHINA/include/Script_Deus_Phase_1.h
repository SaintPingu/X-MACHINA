#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion


#pragma region Class
class Script_Deus_Phase_1 : public Script_Enemy {
	COMPONENT(Script_Deus_Phase_1, Script_Enemy)

protected:
	struct DeusPhase1AttackType : AttackType {
		static constexpr int MeleeAttack = 1;
		static constexpr int RangeAttack = 2;
		static constexpr int ExplodeAttack = 3;
		static constexpr int _count = 4;
	};

public:
	virtual void Awake() override;

protected:
	void MeleeAttackCallback();
	void RangeAttackCallback();
	void ExplodeAttackCallback();
};
#pragma endregion
