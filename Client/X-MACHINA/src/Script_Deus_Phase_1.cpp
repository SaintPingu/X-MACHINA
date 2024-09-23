#include "stdafx.h"
#include "Script_Deus_Phase_1.h"

#include "Script_EnemyManager.h"

#include "AnimatorController.h"
#include "AnimatorMotion.h"

#include "Object.h"
#include "BattleScene.h"

#include "InputMgr.h"
#include "Script_Weapon_MissileLauncher.h"

void Script_Deus_Phase_1::Awake()
{
	base::Awake();

	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack1AnimName)->AddCallback(std::bind(&Script_Deus_Phase_1::MeleeAttackCallback, this), 16);
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack2AnimName)->AddAnimateCallback(std::bind(&Script_Deus_Phase_1::RangeAttackCallback, this));
	mEnemyMgr->mController->FindMotionByName(mEnemyMgr->mStat.Attack3AnimName)->AddAnimateCallback(std::bind(&Script_Deus_Phase_1::ExplodeAttackCallback, this));
}

void Script_Deus_Phase_1::Start()
{
	auto weapon = BattleScene::I->Instantiate("Deus_Phase_1_Rifle_Rust");
	weapon->SetLocalTransform(Matrix::Identity);
	mObject->FindFrame("RefPosGun_R")->SetChild(weapon->GetObj<Transform>()->GetShared());

	mWeapon = weapon->AddComponent<Script_Weapon_Deus_Rifle>();
}

void Script_Deus_Phase_1::StartAttack()
{
	base::StartAttack();

	mWeapon->SetTarget(mEnemyMgr->mTarget);

	switch (mCurrAttackCnt)
	{
	case DeusPhase1AttackType::MeleeAttack:
		std::cout << "Melee" << std::endl;
		break;
	case DeusPhase1AttackType::RangeAttack:
		std::cout << "Range" << std::endl;
		FireMissille();
		break;
	case DeusPhase1AttackType::ExplodeAttack:
		std::cout << "Explode" << std::endl;
		FireMissille();
		break;
	default:
		break;
	}

}

void Script_Deus_Phase_1::MeleeAttackCallback()
{
	std::cout << "Melee" << std::endl;
}

void Script_Deus_Phase_1::RangeAttackCallback()
{
	if (mEnemyMgr->mTarget) {
		mObject->RotateTargetAxisY(mEnemyMgr->mTarget->GetPosition(), mEnemyMgr->mStat.AttackRotationSpeed);
	}
}

void Script_Deus_Phase_1::ExplodeAttackCallback()
{
	if (mEnemyMgr->mTarget) {
		mObject->RotateTargetAxisY(mEnemyMgr->mTarget->GetPosition(), mEnemyMgr->mStat.AttackRotationSpeed);
	}
}

void Script_Deus_Phase_1::FireMissille()
{
	mWeapon->FireBullet();
}