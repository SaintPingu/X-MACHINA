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

private:
	std::string mAttackSound{};
	std::string mDeathSound{};
	std::string mDetectSound{};

protected:
	sptr<Script_EnemyManager>  mEnemyMgr{};

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	virtual void Attack();
	virtual void Death();
	virtual void Dead() override;
	virtual bool Hit(float damage, Object* instigator = nullptr) override;

public:
	void SetEnemyStat(const std::string& modelName);

	void SetAttackSound(const std::string& sound) { mAttackSound = sound; }
	void SetDeathSound(const std::string& sound) { mDeathSound = sound; }
	void SetDetectSound(const std::string& sound) { mDetectSound = sound; }

protected:
	virtual void AttackCallback();
	virtual void AttackEndCallback();
};
#pragma endregion