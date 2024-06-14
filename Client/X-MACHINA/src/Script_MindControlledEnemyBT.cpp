#include "stdafx.h"
#include "Script_MindControlledEnemyBT.h"

#include "Script_Enemy.h"
#include "Script_EnemyManager.h"

#include "CheckMindDetectionRange.h"
#include "CheckAttackRange.h"
#include "CheckPatrolRange.h"
#include "CheckDeath.h"
#include "TaskPatrol.h"
#include "TaskMoveToTarget.h"
#include "TaskAttack.h"
#include "TaskPathPlanningToTarget.h"
#include "TaskPathPlanningToSpawn.h"
#include "TaskMoveToPath.h"
#include "TaskGetHit.h"
#include "TaskMoveToMindControlInvoker.h"


void Script_MindControlledEnemyBT::OnEnable()
{
	base::OnEnable();
	mObject->mObjectCB.MindRimFactor = 1.f;
	mObject->GetComponent<Script_EnemyManager>()->Reset();
}

void Script_MindControlledEnemyBT::OnDisable()
{
	base::OnDisable();
	mObject->mObjectCB.MindRimFactor = 0.f;
}

BT::Node* Script_MindControlledEnemyBT::SetupTree()
{
	auto& enemy = mObject->GetComponent<Script_Enemy>();

#pragma region BehaviorTree
	BT::Node* root = new BT::Selector{ std::vector<BT::Node*>{
		new CheckDeath(mObject, std::bind(&Script_Enemy::Death, enemy)),
		new BT::Sequence{ std::vector<BT::Node*>{
			new CheckAttackRange(mObject),
			new TaskAttack(mObject, std::bind(&Script_Enemy::Attack, enemy))}},
		new TaskGetHit(mObject),
			new BT::Sequence{ std::vector<BT::Node*>{
				new CheckMindDetectionRange(mObject),
				new BT::Selector{ std::vector<BT::Node*>{
					new TaskMoveToMindControlInvoker(mObject, mInvoker),
					new TaskPathPlanningToTarget(mObject)}}}},
		new TaskMoveToPath(mObject)}};
#pragma endregion

	return root;
}