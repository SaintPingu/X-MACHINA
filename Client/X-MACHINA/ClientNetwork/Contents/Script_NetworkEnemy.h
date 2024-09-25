#pragma once
#undef max
#include "ClientNetwork/Include/Protocol/Enum_generated.h"
#include "Script_NetworkLiveObject.h"

class Script_EnemyManager;
class Script_Enemy;

class  Script_NetworkEnemy : public Script_NetworkLiveObject {
	COMPONENT(Script_NetworkEnemy, Script_NetworkLiveObject)

private:
	Script_EnemyManager* mEnemyMgr;
	Script_Enemy* mEnemy;

	float mDeathAccTime{};
	float mDeathRemoveTime{};
	bool mIsDeath{};

public:
	EnemyState GetState();
	Vec3 GetPosition() { return mObject->GetPosition(); }

	void SetPosition(const Vec3& pos);
	void SetRotation(float angle);
	void SetLocalRotation(const Quat& quat);
	void SetState(EnemyState state, int attackCnt);
	void SetState(FBProtocol::MONSTER_BT_TYPE btType, int attackCnt);
	void SetTarget(Object* target);
	void SetCurrAttackCnt(int attackCnt);
	void SetActiveMyObject(bool isActive);

public:
	virtual void Awake() override;
	virtual void Update() override;

private:
	void MoveToTarget();
	void Attack();
	void Idle();
	void Death();
	void GetHit();
};

