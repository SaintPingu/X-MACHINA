#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Collider;
class AnimatorController;
class GameObject;
class Script_Ability_AttackIndicator;
#pragma endregion


#pragma region Class
class Script_MiningMech : public Script_Enemy {
	COMPONENT(Script_MiningMech, Script_Enemy)

protected:
	struct MiningMechAttackType : AttackType {
		static constexpr int DiggerAttack = 1;
		static constexpr int DrillAttack = 2;
		static constexpr int SmashAttack = 3;

		static constexpr int _count = 4;
	};

private:
	int mCurrAttackCnt{};
	wptr<Script_Ability_AttackIndicator> mIndicator{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void LateUpdate() override;

protected:
	void DiggerAttackCallback();
	void DrillAttackCallback();

	void SmashAttackStartCallback();
	void SmashAttackCallback();
	void SmashAttackEndCallback();

	void AttackEndCallback();
};
#pragma endregion

