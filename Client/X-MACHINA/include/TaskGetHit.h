#pragma once


#pragma region Include
#include "Script_BehaviorTree.h"
#pragma endregion


#pragma region ClassForwardDecl
class Script_EnemyManager;
class Script_LiveObject;
#pragma endregion


#pragma region Class
class TaskGetHit : public BT::Node {
private:
	sptr<Script_EnemyManager> mEnemyMgr;
	sptr<Script_LiveObject> mLiveObject;
	float mPrevHp{};
	float mKnockBack{};

public:
	TaskGetHit(Object* object);
	virtual ~TaskGetHit() = default;

public:
	virtual BT::NodeState Evaluate() override;

private:
	void GetHitEndCallback();
};
#pragma endregion


