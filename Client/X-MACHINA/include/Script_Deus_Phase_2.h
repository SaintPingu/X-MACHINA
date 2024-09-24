#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_Enemy.h"
#pragma endregion


#pragma region Class
class Script_Deus_Phase_2 : public Script_Enemy {
	COMPONENT(Script_Deus_Phase_2, Script_Enemy)

protected:
	struct DeusPhase2AttackType : AttackType {
		static constexpr int MeleeAttack = 1;
		static constexpr int _count = 2;
	};

private:
	Object* mLeftWeapon{};
	Object* mRightWeapon{};

public:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void OnDestroy() override;
};
#pragma endregion
