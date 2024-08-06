#include "stdafx.h"
#include "Script_Weapon_MineLauncher.h"

#include "Script_SpiderMine.h"

#include "Object.h"
#include "BattleScene.h"

void Script_Weapon_MineLauncher::Awake()
{
	base::Awake();

	SetFiringMode(FiringMode::SemiAuto);
}

void Script_Weapon_MineLauncher::InitValues()
{
	mMaxFireDelay = 0.1f;	 // 발사 딜레이 거의 없음
	mMaxReloadTime = 999.f;	 // 리로드 없음
	mMaxDistance = 43.f;
	mBulletCntPerMag = mkBulletCntPerMag;
	mMaxMag = 0;

	mErrX = Vec2(-2.f, 2.f);
	mErrY = Vec2(-2.f, 2.f);

	mMuzzle = mObject->FindFrame("FirePos");

	//SetFireSound("Buronout Fire");
}

void Script_Weapon_MineLauncher::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, 0, mkMineSpeed, BulletType::Mine);
	const auto& mine = bullet->GetComponent<Script_SpiderMine>();
	mine->SetExplosionDamage(mkExplosionDamage);
	mine->SetImpactSound("Burnout Impact");
}

void Script_Weapon_MineLauncher::SetParticleSystemNames()
{
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Nuke_Explosion_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Nuke_Explosion_Add");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Nuke_Smoke_Colon");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Nuke_Smoke_Head");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Nuke_Explosion_Mult");
	mPSNames[static_cast<UINT8>(BulletPSType::Contrail)].push_back("WFX_Bullet");
}

void Script_Weapon_MineLauncher::CreateBulletPool()
{
	mBulletPool = BattleScene::I->CreateObjectPool("AT_Mine", mkBulletCntPerMag, std::bind(&Script_Weapon_MineLauncher::BulletInitFunc, this, std::placeholders::_1));
}
