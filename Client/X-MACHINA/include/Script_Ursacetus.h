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


#pragma region Enum
enum class AttackType {
	BasicAttack = 0,	// 0~2번 공격은 기본 공격
	RoarAttack = 3,		// 3번 공격은 Roar 공격
	SpecialAttack = 4,	// 4번 공격은 특별 공격

	_count,
};
enum { AttackTypeCount = static_cast<UINT8>(AttackType::_count) };
#pragma endregion


#pragma region Class
class Script_Ursacetus : public Script_Enemy {
	COMPONENT(Script_Ursacetus, Script_Enemy)

private:
	int mCurrAttackCnt{};
	Transform* mLeftForeArm{};
	sptr<Collider> mArmCollider{};
	wptr<Script_TriggerAbilityHolder> mSpecialAttack{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Attack() override;

protected:
	void BasicAttackCallback();
	void RoarAttackCallback();
	
	void SpecialAttack();
	void SpecialAttackCallback();
	void SpecialAttackStartCallback();

	virtual void AttackEndCallback() override;
};
#pragma endregion