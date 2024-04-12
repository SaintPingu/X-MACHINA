#include "stdafx.h"
#include "Script_Weapon_Rifle.h"



#pragma region Script_Weapon_Rifle
void Script_Weapon_Rifle::Awake()
{
	base::Awake();

	SetFiringMode(FiringMode::Auto);
}
#pragma endregion





#pragma region Script_Weapon_Skyline
void Script_Weapon_Skyline::InitValues()
{
	mMaxFireDelay  = CalcFireDelay(mkRPM);
	mMaxReloadTime = 2.1f;
	mMaxDistance   = 35.f;
	mMaxMag        = 35;
}

void Script_Weapon_Skyline::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed);
}
#pragma endregion

