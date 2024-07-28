#pragma once

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
	void SetRotation(const Vec3& rot);
	void SetState(EnemyState state);
	void SetTarget(Object* target);
	EnemyState GetState();

public:
	void MoveToTarget();
	void Attack();
};

