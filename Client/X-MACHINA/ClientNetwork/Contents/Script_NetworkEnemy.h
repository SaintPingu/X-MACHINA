#pragma once
#undef max
#include "ClientNetwork/Include/Protocol/Enum_generated.h"
#include "Script_NetworkLiveObject.h"

class Script_EnemyManager;

class  Script_NetworkEnemy : public Script_NetworkLiveObject {
	COMPONENT(Script_NetworkEnemy, Script_NetworkLiveObject)
private:
	Script_EnemyManager* mEnemyMgr;
	float mDeathAccTime{};
	float mDeathRemoveTime{};
	//std::vector<PheroInfo> mPheros;

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	void SetPosition(const Vec3& pos);
	void SetRotation(float angle);
	void SetLocalRotation(const Quat& quat);
	void SetState(EnemyState state);
	void SetState(FBProtocol::MONSTER_BT_TYPE btType);
	void SetTarget(Object* target);
	void SetActiveMyObject(bool isActive);
	EnemyState GetState();

	Vec3 GetPosition() { return mObject->GetPosition(); }
	//void SetPheroInfo(std::vector<PheroInfo> pheros) { mPheros = pheros; }

public:
	void MoveToTarget();
	void Attack();
	void Idle();
	void Death();
};

