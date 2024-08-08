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
class Script_Ursacetus : public Script_Enemy {
	COMPONENT(Script_Ursacetus, Script_Enemy)

protected:
	struct UrsacetusAttackType : AttackType {
		static constexpr int RoarAttack = 4;
		static constexpr int SpecialAttack = 5;

		static constexpr int _count = 6;
	};

private:
	Transform* mLeftForeArm{};
	sptr<Collider> mArmCollider{};
	wptr<Script_Ability_AttackIndicator> mIndicator{};

public:
	virtual void Awake() override;
	virtual void Start() override;

protected:
	void RoarAttackCallback();
	
	void SpecialAttackCallback();
	void SpecialAttackStartCallback();

	virtual void AttackEndCallback() override;
};
#pragma endregion