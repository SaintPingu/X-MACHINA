#include "stdafx.h"
#include "Script_Ursacetus.h"

#include "Script_EnemyManager.h"
#include "Script_MainCamera.h"
#include "Script_PheroObject.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

#include "Script_Ability_AttackIndicator.h"

#include "Component/ParticleSystem.h"
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
		
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack2AnimName)->AddCallback(std::bind(&Script_Ursacetus::RoarAttackCallback, this), 43);

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddCallback(std::bind(&Script_Ursacetus::SpecialAttackCallback, this), 65);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddStartCallback(std::bind(&Script_Ursacetus::SpecialAttackStartCallback, this));

	SetDetectSound("Ursacetus Detect");
	SetAttackSound("Ursacetus Attack1");
	SetDeathSound("Ursacetus Death");
}

void Script_Ursacetus::Start()
{
	base::Start();

	mCurrAttackCnt = static_cast<int>(AttackType::BasicAttack);

	mIndicator = mObject->AddComponent<Script_Ability_AttackIndicator>();
	mIndicator.lock()->Init(2.05f, "CircleIndicator");
}


void Script_Ursacetus::RoarAttackCallback()
{
	mObject->mObjectCB.MindRimFactor = 1.f;
	mObject->mObjectCB.MindRimColor = Vec3{ 1.f, 0.f, 0.f };
	MainCamera::I->GetComponent<Script_MainCamera>()->StartShake(1.f, 0.003f);
}

void Script_Ursacetus::SpecialAttackCallback()
{
	mObject->mObjectCB.MindRimFactor = 0.f;
	mObject->mObjectCB.MindRimColor = Vec3{ 0.5f, 0.f, 0.5f };
	MainCamera::I->GetComponent<Script_MainCamera>()->StartShake(2.f, 0.006f);

	mIndicator.lock()->SetActive(false);
	ParticleManager::I->Play("Ursacetus_Smash_Dust_Ring", mObject->FindFrame("Ursacetus_ L Toe0"));
	ParticleManager::I->Play("Ursacetus_Smash_Dust_Spread", mObject->FindFrame("Ursacetus_ L Toe0"));
}

void Script_Ursacetus::SpecialAttackStartCallback()
{
	//if (mEnemyMgr->mController->GetParamValue<int>("Attack") > 0) {
		mIndicator.lock()->SetActive(true);
	//}
}

void Script_Ursacetus::AttackEndCallback()
{
	if (mEnemyMgr->mState != EnemyState::Attack) {
		mEnemyMgr->mController->SetValue("Attack", UrsacetusAttackType::None);
		return;
	}

	mEnemyMgr->mController->SetValue("Attack", mCurrAttackCnt);

	if (mCurrAttackCnt >= UrsacetusAttackType::BasicAttack) {
		mEnemyMgr->mController->SetValue("Attack", UrsacetusAttackType::None);
		mEnemyMgr->mState = EnemyState::Idle;
	}
}