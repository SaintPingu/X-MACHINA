#include "stdafx.h"
#include "Script_Weapon_Pistol.h"


void Script_Weapon_Pistol::Awake()
{
	base::Awake();

	SetFiringMode(FiringMode::SemiAuto);
}

void Script_Weapon_Pistol::InitValues()
{
	mMaxFireDelay     = CalcFireDelay(mkRPM);
	mMaxReloadTime    = 1.7f;
	mMaxDistance      = 16.f;
	mBulletCntPerMag  = 15;
	mMaxMag           = 8;
}

void Script_Weapon_Pistol::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed);
}

void Script_Weapon_Pistol::SetParticleSystemNames()
{
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Smoke_Building");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Nuke_Explosion_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Nuke_Explosion_Add");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Nuke_Smoke_Colon");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Nuke_Smoke_Head");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Nuke_Explosion_Mult");
	mPSNames[static_cast<UINT8>(BulletPSType::Contrail)].push_back("WFX_Bullet");
}
