#pragma once
#undef max
#include "ClientNetwork/Include/Protocol/Enum_generated.h"
#include "Script_NetworkLiveObject.h"

class Script_EnemyManager;

class  Script_NetworkEnemy  : public Script_NetworkLiveObject {
	COMPONENT(Script_NetworkEnemy, Script_NetworkLiveObject)
private:
	Script_EnemyManager* mEnemyMgr;

public:
	virtual void Awake() override;
	virtual void Update() override;

public:
	void SetPostion(const Vec3& pos);
	void SetRotation(float angle);
	void SetState(EnemyState state);
	void SetState(FBProtocol::MONSTER_BT_TYPE btType);
	void SetTarget(Object* target);
	void SetActiveMyObject(bool isActive);
	EnemyState GetState();

public:
	void MoveToTarget();
	void Attack();
};

