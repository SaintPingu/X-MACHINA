#include "stdafx.h"
#include "Script_Ursacetus.h"

#include "Script_EnemyManager.h"
#include "Script_MainCamera.h"
#include "Script_PheroObject.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

#include "Script_AbilityHolder.h"
#include "CircleIndicator.h"
#include "IRDetectorAbility.h"

#include "Component/Collider.h"
#include "Component/Camera.h"
#include "Object.h"

#include "ResourceMgr.h"
#include "Mesh.h"

void Script_Ursacetus::Awake()
{
	base::Awake();

	mLeftForeArm = mObject->FindFrame("Ursacetus_ L Forearm", true);
	mArmCollider = mLeftForeArm->GetObj<Object>()->GetComponent<Collider>(true);
		
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Ursacetus::BasicAttackCallback, this), 57);

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack2AnimName)->AddCallback(std::bind(&Script_Ursacetus::RoarAttackCallback, this), 43);

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddAnimateCallback(std::bind(&Script_Ursacetus::SpecialAttack, this));
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddCallback(std::bind(&Script_Ursacetus::SpecialAttackCallback, this), 65);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddCallback(std::bind(&Script_Ursacetus::SpecialAttackStartCallback, this), 10);
}

void Script_Ursacetus::Start()
{
	base::Start();

	mSpecialAttack = mObject->AddComponent<Script_TriggerAbilityHolder>();
	mSpecialAttack.lock()->SetAbility(0, std::make_shared<CircleIndicator>(2.05f));
}

void Script_Ursacetus::Attack()
{
	mEnemyMgr->RemoveAllAnimation();
	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt);
}

void Script_Ursacetus::BasicAttackCallback()
{
	if (!mEnemyMgr->mTarget) {
		return;
	}

	const float dis = Vec3::Distance(mEnemyMgr->mTarget->GetPosition(), mObject->GetPosition());
	if (mArmCollider->Intersects(mEnemyMgr->mTarget->GetComponent<Collider>()) || dis < 3.f) {
		auto liveObject = mEnemyMgr->mTarget->GetComponent<Script_LiveObject>();
		if (liveObject) {
			liveObject->Hit(mEnemyMgr->mStat.AttackRate, mObject);
		}
	}
}


void Script_Ursacetus::RoarAttackCallback()
{
	mObject->mObjectCB.MindRimFactor = 1.f;
	mObject->mObjectCB.MindRimColor = Vec3{ 1.f, 0.f, 0.f };
	MainCamera::I->GetComponent<Script_MainCamera>()->StartShake(1.f, 0.003f);
}

void Script_Ursacetus::SpecialAttack()
{
}

void Script_Ursacetus::SpecialAttackCallback()
{
	// 거리에 따른 데미지
	const float specialAttackRange = mEnemyMgr->mStat.AttackRange * 2.5f;
	const float specialAttackRate = mEnemyMgr->mStat.AttackRate * 2.f;
	const float dis = Vec3::Distance(mEnemyMgr->mTarget->GetPosition(), mObject->GetPosition());
	if (dis <= specialAttackRange) {
		const float damageRatio = 1.f - dis / specialAttackRange;	// 거리에 따른 데미지 비율
		auto liveObject = mEnemyMgr->mTarget->GetComponent<Script_LiveObject>();
		if (liveObject) {
			liveObject->Hit(specialAttackRate * damageRatio, mObject);
		}
	}

	mObject->mObjectCB.MindRimFactor = 0.f;
	mObject->mObjectCB.MindRimColor = Vec3{ 0.5f, 0.f, 0.5f };
	MainCamera::I->GetComponent<Script_MainCamera>()->StartShake(2.f, 0.006f);

	mSpecialAttack.lock()->OnTrigger(true);
}

void Script_Ursacetus::SpecialAttackStartCallback()
{
	mSpecialAttack.lock()->OnTrigger(true);
}

void Script_Ursacetus::AttackEndCallback()
{
	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt++);
	mEnemyMgr->mState = EnemyState::Idle;

	mCurrAttackCnt %= AttackTypeCount;
}
