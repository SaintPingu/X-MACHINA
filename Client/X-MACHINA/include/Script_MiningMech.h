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
class Script_TriggerAbilityHolder;
#pragma endregion


#pragma region Class
class Script_MiningMech : public Script_Enemy {
	COMPONENT(Script_MiningMech, Script_Enemy)

	enum class AttackType {
		DiggerAttack = 0,
		DrillAttack = 1,
		SmashAttack = 2,

		_count,
	};
	enum { AttackTypeCount = static_cast<UINT8>(AttackType::_count) };

private:
	int mCurrAttackCnt{};
	wptr<Script_TriggerAbilityHolder> mCircleIndicator{};

public:
	virtual void Awake() override;
	virtual void Attack() override;

protected:
	void DiggerAttackCallback();
	void DrillAttackCallback();
	void SmashAttackCallback();

	void AttackEndCallback();
};
#pragma endregion

