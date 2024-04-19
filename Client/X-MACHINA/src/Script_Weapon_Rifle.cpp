#include "stdafx.h"
#include "Script_Weapon_Rifle.h"


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

	mCurErr.x += mErrIncAmount;
	mCurErr.y += mErrIncAmount;

	mCurErr.x = std::clamp(mCurErr.x, -mMaxErrX, mMaxErrX);
	mCurErr.y = std::clamp(mCurErr.y, -mMaxErrY, mMaxErrY);

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
#pragma endregion

