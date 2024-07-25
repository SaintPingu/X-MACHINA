#pragma once

#include "Script_Network.h"

class Script_EnemyManager;

class Script_EnemyNetwork : public Script_Network {
	COMPONENT(Script_EnemyNetwork, Script_Network)
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

public:
	void MoveToTarget();
};

