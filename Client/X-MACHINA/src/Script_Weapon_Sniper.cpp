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
}

void Script_Weapon_PipeLine::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed);
}