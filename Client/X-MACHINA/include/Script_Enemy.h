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
	struct AttackType {
		static constexpr int None = 0;
		static constexpr int BasicAttack = 1;

		static constexpr int _count = 2;
	};
	int mCurrAttackCnt{};
	bool mIgnoreAttackEndCallback{};

private:
	std::string mAttackSound{};
	std::string mDeathSound{};
	std::string mDetectSound{};

	bool mNoTarget{ true };

protected:
	sptr<Script_EnemyManager>  mEnemyMgr{};

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	virtual void StartAttack();
	virtual void Attack();
	virtual void GetHit();
	virtual void Dead() override;
	virtual void Detect();
	virtual bool Hit(float damage, Object* instigator = nullptr) override;
	void IgnoreAttackEndCallback() { mIgnoreAttackEndCallback = true; }

public:
	int GetCurrAttackCnt() const { return mCurrAttackCnt; }

public:
	void SetEnemyStat(const std::string& modelName);
	void SetCurrAttackCnt(int attackCnt);

	void SetAttackSound(const std::string& sound) { mAttackSound = sound; }
	void SetDeathSound(const std::string& sound) { mDeathSound = sound; }
	void SetDetectSound(const std::string& sound) { mDetectSound = sound; }

protected:
	virtual void AttackCallback();
	virtual void AttackEndCallback();
	virtual void DeathEndCallback();
};
#pragma endregion