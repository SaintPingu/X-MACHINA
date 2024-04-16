#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
#pragma endregion


#pragma region Class
class CheckPatrolRange : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr{};
	
	Vec3 mSpawnPos{};
	Vec3 mBaryCenter{};
	float mPatrolRange{};

public:
	CheckPatrolRange(Object* object, const Vec3& baryCenter, float waySize);
	virtual ~CheckPatrolRange() = default;

	virtual BT::NodeState Evaluate() override;
};
#pragma endregion
