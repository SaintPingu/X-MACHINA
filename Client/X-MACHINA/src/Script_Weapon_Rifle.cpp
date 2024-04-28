#include "stdafx.h"
#include "Script_Weapon_Rifle.h"
#include "Component/ParticleSystem.h"

#include "Timer.h"



#pragma region Script_Weapon_Rifle
void Script_Weapon_Rifle::Awake()
{
	base::Awake();

	SetFiringMode(FiringMode::Auto);
}

void Script_Weapon_Rifle::Update()
{
	base::Update();

	// ÅºÆÛÁü ¿ÏÈ­
	if (!IsShooting()) {
		auto RecoverErr = [this](float& err) {
			if (err > 0) {
				err -= mErrRecoverSpeed * DeltaTime();
				if (err < 0) {
					err = 0;
				}
			}
			};

		RecoverErr(mCurErr.x);
		RecoverErr(mCurErr.y);
	}
}

void Script_Weapon_Rifle::FireBullet()
{
	base::FireBullet();

	// ÇöÀç ÅºÆÛÁü Áõ°¡
	mCurErr.x += mErrIncAmount;
	mCurErr.y += mErrIncAmount;

	// ÃÖ´ñ°ª Á¦ÇÑ
	mCurErr.x = std::clamp(mCurErr.x, -mMaxErrX, mMaxErrX);
	mCurErr.y = std::clamp(mCurErr.y, -mMaxErrY, mMaxErrY);
	
	// °ª Àû¿ë
	mErrX = Vec2(-mCurErr.x, mCurErr.x);
	mErrY = Vec2(-mCurErr.y, mCurErr.y);
}
#pragma endregion





#pragma region Script_Weapon_Skyline
void Script_Weapon_Skyline::InitValues()
{
	mMaxFireDelay     = CalcFireDelay(mkRPM);
	mMaxReloadTime    = 2.1f;
	mMaxDistance      = 35.f;
	mBulletCntPerMag  = 35;
	mMaxMag           = 6;

	mMaxErrX = 8.f;
	mMaxErrX = 3.f;
}

void Script_Weapon_Skyline::BulletInitFunc(rsptr<InstObject> bullet) const
{
	base::InitBullet(bullet, mkBulletDamage, mkBulletSpeed);
}

//void Script_Weapon_Skyline::SetParticleSystemNames()
//{
//	mPSNames[static_cast<UINT8>(BulletPSType::Building)].push_back("WFX_Smoke_Building");
//	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Smoke_BigQuick");
//	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Smoke");
//	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Explosion_Small");
//	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Dot_Sparkles");
//	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Dot_Sparkles_Big");
//	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Dot_Sparkles_Mult");
//	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Glow");
//	mPSNames[static_cast<UINT8>(BulletPSType::Explosion)].push_back("WFX_Explosion");
//	mPSNames[static_cast<UINT8>(BulletPSType::Contrail)].push_back("WFX_Bullet");
//}

#pragma endregion

