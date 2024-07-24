#pragma once


#pragma region Include
#include "Component/Component.h"
#include "Script_LiveObject.h"
#pragma endregion


#pragma region ClassForwardDecl
class GridObject;
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class Script_Enemy abstract : public Script_LiveObject {
	COMPONENT_ABSTRACT(Script_Enemy, Script_LiveObject)

protected:
	sptr<Script_EnemyManager>  mEnemyMgr{};

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	virtual void Attack();
	virtual void Death();
	virtual bool Hit(float damage, Object* instigator = nullptr) override;

public:
	void SetEnemyStat(const std::string& modelName);

protected:
	virtual void AttackCallback();
	virtual void AttackEndCallback();
};
#pragma endregion