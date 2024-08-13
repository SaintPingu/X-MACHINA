#include "stdafx.h"
#include "Script_Weapon_Sniper.h"

void Script_Weapon_Sniper::Awake()
{
	base::Awake();

	SetFiringMode(FiringMode::BoltAction);
}



void Script_Weapon_PipeLine::InitValues()
{
	mMaxFireDelay     = CalcFireDelay(mkRPM);
	mMaxReloadTime    = 4.1F;
	mMaxDistance      = 60.f;
	mBulletCntPerMag  = 5;
	mMaxMag           = 5;

	SetFireSound("PipeLine Fire");
}

void Script_Weapon_PipeLine::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed);
}

void Script_Weapon_PipeLine::SetParticleSystemNames()
{
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Smoke_BigQuick");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Dot_Sparkles");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Glow");
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Explosion");

	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Smoke_BigQuick");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Dot_Sparkles");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Glow");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Explosion");

	mPSNames[static_cast<UINT8>(BulletPSType::Contrail)].push_back("WFX_Bullet");
}
