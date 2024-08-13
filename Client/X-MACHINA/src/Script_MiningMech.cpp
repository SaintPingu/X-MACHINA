#include "stdafx.h"
#include "Script_MiningMech.h"

#include "Script_EnemyManager.h"
#include "Script_MainCamera.h"

#include "Component/Collider.h"
#include "Component/Camera.h"

#include "Script_Ability_AttackIndicator.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"


void Script_MiningMech::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_MiningMech::AttackCallback, this), 15);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack2AnimName)->AddCallback(std::bind(&Script_MiningMech::AttackCallback, this), 15);

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddStartCallback(std::bind(&Script_MiningMech::SmashAttackStartCallback, this));
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddCallback(std::bind(&Script_MiningMech::SmashAttackCallback, this), 20);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddCallback(std::bind(&Script_MiningMech::SmashAttackEndCallback, this), 49);
}

void Script_MiningMech::Start()
{
	base::Start();

	mIndicator = mObject->AddComponent<Script_Ability_AttackIndicator>(true, false);
	mIndicator.lock()->Init(1.8f, "RectangleIndicator");
}

void Script_MiningMech::DiggerAttackCallback()
{
}

void Script_MiningMech::DrillAttackCallback()
{
}

void Script_MiningMech::SmashAttackStartCallback()
{
	mIndicator.lock()->SetActive(true);
}

void Script_MiningMech::SmashAttackCallback()
{
	MyBoundingOrientedBox box;
	box.Center = mObject->GetPosition();
	box.Extents = Vec3{ 3.5f, 10.f, 7.f };
	box.Transform(mObject->GetWorldTransform());

	if (mEnemyMgr->mTarget->GetComponent<Collider>()->Intersects(box)) {
		auto liveObject = mEnemyMgr->mTarget->GetComponent<Script_LiveObject>();
		if (liveObject) {
			liveObject->Hit(mEnemyMgr->mStat.AttackRate, mObject);
		}
	}
}

void Script_MiningMech::SmashAttackEndCallback()
{
	mIndicator.lock()->SetActive(false);
}
