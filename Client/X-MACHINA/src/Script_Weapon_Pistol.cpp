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