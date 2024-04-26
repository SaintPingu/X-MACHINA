#include "stdafx.h"
#include "Script_Weapon_Rifle.h"
#include "Component/ParticleSystem.h"


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

void Script_Weapon_Skyline::SetParticleSystemNames()
{
	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Smoke_Building");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Smoke_BigQuick");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Smoke");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Explosion_Small");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Dot_Sparkles");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Dot_Sparkles_Big");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Dot_Sparkles_Mult");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Glow");
	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Explosion");
	mPSNames[static_cast<UINT8>(BulletPSType::Contrail)].push_back("WFX_Bullet");
}

#pragma endregion

